/* https://github.com/takeiteasy/hal

 hal Copyright (C) 2025 George Watson

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>. */

// iOS flash using AVCaptureDevice

#ifndef HAL_NO_FLASH
#include "hal/flash.h"
#import <AVFoundation/AVFoundation.h>

static bool flash_state = false;

bool hal_flash_available(void) {
    AVCaptureDevice *device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    return device && [device hasTorch];
}

void hal_flash_on(void) {
    AVCaptureDevice *device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    if (!device || ![device hasTorch])
        return;
    
    NSError *error = nil;
    if ([device lockForConfiguration:&error]) {
        [device setTorchMode:AVCaptureTorchModeOn];
        [device unlockForConfiguration];
        flash_state = true;
    }
}

void hal_flash_off(void) {
    AVCaptureDevice *device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    if (!device || ![device hasTorch])
        return;
    
    NSError *error = nil;
    if ([device lockForConfiguration:&error]) {
        [device setTorchMode:AVCaptureTorchModeOff];
        [device unlockForConfiguration];
        flash_state = false;
    }
}

bool hal_flash_toggle(void) {
    if (flash_state) {
        hal_flash_off();
    } else {
        hal_flash_on();
    }
    return flash_state;
}
#endif // HAL_NO_FLASH
