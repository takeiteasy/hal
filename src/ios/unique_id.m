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

// iOS unique ID using identifierForVendor

#ifndef HAL_NO_UNIQUE_ID
#include "hal/unique_id.h"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include <string.h>

static char unique_id_buffer[64] = {0};

bool hal_unique_id_available(void) {
    return true;
}

const char* hal_unique_id_get(void) {
    NSUUID *uuid = [[UIDevice currentDevice] identifierForVendor];
    if (!uuid)
        return NULL;
    
    NSString *uuidString = [uuid UUIDString];
    if (!uuidString)
        return NULL;
    
    const char *utf8 = [uuidString UTF8String];
    if (utf8) {
        strncpy(unique_id_buffer, utf8, sizeof(unique_id_buffer) - 1);
        unique_id_buffer[sizeof(unique_id_buffer) - 1] = '\0';
        return unique_id_buffer;
    }
    
    return NULL;
}
#endif // HAL_NO_UNIQUE_ID
