#include "annepro2.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

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

    // Первый поиск
    devices = hid_enumerate(ANNEPRO2_VID, 0);
    for (dev = devices; dev; dev = dev->next) {
        if ((dev->product_id == PID_C15 && dev->interface_number == 1) ||
            dev->product_id == PID_C18) {
            handle = hid_open_path(dev->path);
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

    // Get firmware size
    fseek(file, 0, SEEK_END);
    size_t total_size = ftell(file);
    rewind(file);

    uint8_t chunk[48];
    size_t read_size;
    uint32_t current_addr = base;
    size_t total_written = 0;
    time_t start = time(NULL);

    while ((read_size = fread(chunk, 1, sizeof(chunk), file)) > 0) {
        uint8_t flash_cmd[6 + 48] = {
            0x02, 0x31,
            current_addr & 0xFF, (current_addr >> 8) & 0xFF,
            (current_addr >> 16) & 0xFF, (current_addr >> 24) & 0xFF
        };
        memcpy(flash_cmd + 6, chunk, read_size);

        if (write_to_target(handle, target, flash_cmd, 6 + read_size) < 0) {
            fprintf(stderr, "\nError flashing memory at address 0x%08x.\n", current_addr);
            hid_close(handle);
            return FlashError;
        }

        total_written += read_size;
        current_addr += read_size;

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
