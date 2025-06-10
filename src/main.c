// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "annepro2.h"

uint32_t parse_hex(const char *str) {
    if (strncmp(str, "0x", 2) == 0 || strncmp(str, "0X", 2) == 0) {
        return (uint32_t)strtoul(str + 2, NULL, 16);
    }
    return (uint32_t)strtoul(str, NULL, 16);
}

void print_usage(const char *prog) {
    printf("Usage: %s [--base <hex>] [--boot] [-t <target>] <file>\n", prog);
    printf("Targets: main, led, ble\n");
}

int main(int argc, char *argv[]) {
    uint32_t base = 0x4000;
    int boot = 0;
    const char *target_str = "main";
    const char *file_path = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--base") == 0 && i + 1 < argc) {
            base = parse_hex(argv[++i]);
        } else if (strcmp(argv[i], "--boot") == 0) {
            boot = 1;
        } else if ((strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--target") == 0) && i + 1 < argc) {
            target_str = argv[++i];
        } else if (argv[i][0] != '-') {
            file_path = argv[i];
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (!file_path) {
        print_usage(argv[0]);
        return 1;
    }

    AP2Target target;
    if (strcasecmp(target_str, "main") == 0) {
        target = McuMain;
    } else if (strcasecmp(target_str, "led") == 0) {
        target = McuLed;
    } else if (strcasecmp(target_str, "ble") == 0) {
        target = McuBle;
    } else {
        fprintf(stderr, "Invalid target: %s\n", target_str);
        return 1;
    }

    FILE *f = fopen(file_path, "rb");
    if (!f) {
        perror("Failed to open file");
        return 1;
    }

    int result = flash_firmware(target, base, f, boot);
    fclose(f);

    if (result == 0) {
        printf("Flash complete\n");
        if (boot) {
            printf("Booting Keyboard\n");
        }
    } else {
        fprintf(stderr, "Flash error: %s\n", get_flash_error_msg(result));
        return 1;
    }

    return 0;
}

