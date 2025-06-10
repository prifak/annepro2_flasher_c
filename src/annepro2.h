// annepro2.h
#ifndef ANNEPRO2_H
#define ANNEPRO2_H

#include <stdint.h>
#include <stdio.h>
#include "hidapi.h"

typedef enum {
    UsbHost = 1,
    BleHost = 2,
    McuMain = 3,
    McuLed = 4,
    McuBle = 5
} AP2Target;

typedef enum {
    NoDeviceFound,
    MultipleDeviceFound,
    USBError,
    EraseError,
    FlashError,
    OtherError
} AP2FlashError;

int flash_firmware(AP2Target target, uint32_t base, FILE *file, int boot);
const char* get_flash_error_msg(AP2FlashError err);

#endif

