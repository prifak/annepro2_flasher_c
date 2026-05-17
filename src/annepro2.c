#include "annepro2.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#define sleep(x) Sleep(1000 * (x))
#else
#include <unistd.h>
#endif

#define ANNEPRO2_VID 0x04d9
#define PID_C15 0x8008
#define PID_C18 0x8009

static int write_to_target(hid_device *handle, AP2Target target, const uint8_t *payload, size_t payload_len);

static const char *flash_error_msgs[] = {
    "No device found",
    "Multiple devices found",
    "USB error",
    "Erase error",
    "Flash error",
    "Unknown error"
};

const char* get_flash_error_msg(AP2FlashError err) {
    if (err < 0 || err >= OtherError) return flash_error_msgs[OtherError];
    return flash_error_msgs[err];
}

int flash_firmware(AP2Target target, uint32_t base, FILE *file, int boot) {
    hid_device *handle = NULL;
    struct hid_device_info *devices = NULL, *dev = NULL;

    // First search
    devices = hid_enumerate(ANNEPRO2_VID, 0);
    for (dev = devices; dev; dev = dev->next) {
        if ((dev->product_id == PID_C15 && dev->interface_number == 1) ||
            dev->product_id == PID_C18) {

            handle = hid_open_path(dev->path);
        if (!handle) {
            fprintf(stderr, "Error: Device found, but permission denied. Run as root or check udev rules.\n");
            hid_free_enumeration(devices);
            return USBError;
        }
        break;
            }
    }
    hid_free_enumeration(devices);

    // Device waiting
    if (!handle) {
        printf("Please put your keyboard into IAP mode (hold ESC while plugging it in).\n");
        for (int i = 10; i > 0; i--) {
            sleep(1);
            devices = hid_enumerate(ANNEPRO2_VID, 0);
            for (dev = devices; dev; dev = dev->next) {
                if ((dev->product_id == PID_C15 && dev->interface_number == 1) ||
                    dev->product_id == PID_C18) {

                    handle = hid_open_path(dev->path);
                if (!handle) {
                    fprintf(stderr, "Error: Device found, but permission denied. Run as root or check udev rules.\n");
                    hid_free_enumeration(devices);
                    return USBError;
                }
                break;
                    }
            }
            hid_free_enumeration(devices);
            if (handle) break;
            printf("Waiting... %ds remaining\r", i);
            fflush(stdout);
        }
        printf("\n");
    }

    if (!handle) {
        fprintf(stderr, "No compatible device found after waiting. Ensure the keyboard is in IAP mode.\n");
        return NoDeviceFound;
    }

    // Erasing
    uint8_t erase_cmd[6] = {
        0x02, 0x43,
        base & 0xFF, (base >> 8) & 0xFF,
        (base >> 16) & 0xFF, (base >> 24) & 0xFF
    };
    if (write_to_target(handle, target, erase_cmd, sizeof(erase_cmd)) < 0) {
        fprintf(stderr, "Error erasing device memory.\n");
        hid_close(handle);
        return EraseError;
    }

    if (write_to_target(handle, target, erase_cmd, sizeof(erase_cmd)) < 0) {
        fprintf(stderr, "Error erasing device memory.\n");
        hid_close(handle);
        return EraseError;
    }

    // Loading to RAM
    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "Error: Cannot seek file. Is it a regular file?\n");
        hid_close(handle);
        return FlashError;
    }

    long size = ftell(file);
    if (size <= 0) {
        fprintf(stderr, "Error: Firmware file is empty or invalid.\n");
        hid_close(handle);
        return FlashError;
    }
    size_t total_size = (size_t)size;
    rewind(file);

    uint8_t *file_buffer = malloc(total_size);
    if (!file_buffer) {
        fprintf(stderr, "Error: Failed to allocate memory for firmware.\n");
        hid_close(handle);
        return OtherError;
    }

    if (fread(file_buffer, 1, total_size, file) != total_size) {
        fprintf(stderr, "Error: Failed to read file into memory.\n");
        free(file_buffer);
        hid_close(handle);
        return FlashError;
    }

    // flash from RAM (eliminate disk reads
    uint32_t current_addr = base;
    size_t total_written = 0;
    time_t start = time(NULL);

    while (total_written < total_size) {
        size_t chunk_size = (total_size - total_written > 48) ? 48 : (total_size - total_written);

        uint8_t flash_cmd[6 + 48] = {
            0x02, 0x31,
            current_addr & 0xFF, (current_addr >> 8) & 0xFF,
            (current_addr >> 16) & 0xFF, (current_addr >> 24) & 0xFF
        };

        memcpy(flash_cmd + 6, file_buffer + total_written, chunk_size);

        if (write_to_target(handle, target, flash_cmd, 6 + chunk_size) < 0) {
            fprintf(stderr, "\nError flashing memory at address 0x%08x.\n", current_addr);
            free(file_buffer); 
            hid_close(handle);
            return FlashError;
        }

        total_written += chunk_size;
        current_addr += chunk_size;

        float percent = (float)total_written / total_size;
        int bars = (int)(percent * 30);
        time_t now = time(NULL);
        int elapsed = (int)difftime(now, start);

        printf("\r[");
        for (int i = 0; i < 30; i++) {
            if (i < bars) printf("=");
            else printf(" ");
        }
        printf("] %3.0f%% | %zu/%zu bytes | %ds elapsed", percent * 100, total_written, total_size, elapsed);
        fflush(stdout);
    }

    free(file_buffer);

    printf("\nFlash complete: %zu bytes written in %lds.\n", total_written, time(NULL) - start);
    
    // Set AP FLAG
    uint8_t ap_flag_cmd[3] = {0x02, 0x32, 0x02};
    if (write_to_target(handle, McuMain, ap_flag_cmd, sizeof(ap_flag_cmd)) < 0) {
        fprintf(stderr, "Error writing AP flag.\n");
        hid_close(handle);
        return FlashError;
    }

    // Device restart
    if (boot) {
        uint8_t boot_cmd[] = {
            0x00, 0x7b, 0x10, 0x31,
            0x10, 0x03, 0x00, 0x00,
            0x7d, 0x02, 0x01, 0x02
        };
        if (hid_write(handle, boot_cmd, sizeof(boot_cmd)) < 0) {
            fprintf(stderr, "Error booting device.\n");
            hid_close(handle);
            return FlashError;
        }
    }

    hid_close(handle);
    return 0;
}

static int write_to_target(hid_device *handle, AP2Target target, const uint8_t *payload, size_t payload_len) {
    // file size - 55 bytes
    if (payload_len > 55) {
        fprintf(stderr, "CRITICAL: payload_len (%zu) exceeds HID packet size!\n", payload_len);
        return -1;
    }
    uint8_t buffer[64] = {0};
    buffer[1] = 0x7b;
    buffer[2] = 0x10;
    buffer[3] = ((target & 0x0F) << 4) | UsbHost;
    buffer[4] = 0x10;
    buffer[5] = (uint8_t)payload_len;
    buffer[6] = 0x00;
    buffer[7] = 0x00;
    buffer[8] = 0x7d;
    memcpy(buffer + 9, payload, payload_len);

    // Padding
    memset(buffer + 9 + payload_len, 0, 64 - 9 - payload_len);

    if (hid_write(handle, buffer, sizeof(buffer)) < 0) {
        fprintf(stderr, "HID write error.\n");
        return -1;
    }

    uint8_t response[64] = {0};
    if (hid_read(handle, response, sizeof(response)) < 0) {
        fprintf(stderr, "HID read error.\n");
        return -1;
    }

    return 0;
}
