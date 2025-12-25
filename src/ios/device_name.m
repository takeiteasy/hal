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

// iOS device name using UIDevice

#ifndef HAL_NO_DEVICE_NAME
#include "hal/device_name.h"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

static char device_name_buffer[256] = {0};

bool hal_device_name_available(void) {
    return true;
}

const char* hal_device_name_get(void) {
    NSString *name = [[UIDevice currentDevice] name];
    if (name) {
        const char *utf8 = [name UTF8String];
        if (utf8) {
            strncpy(device_name_buffer, utf8, sizeof(device_name_buffer) - 1);
            device_name_buffer[sizeof(device_name_buffer) - 1] = '\0';
            return device_name_buffer;
        }
    }
    return NULL;
}
#endif // HAL_NO_DEVICE_NAME
