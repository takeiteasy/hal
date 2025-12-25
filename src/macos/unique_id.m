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

// macOS unique ID using IOPlatformSerialNumber via IOKit

#ifndef HAL_NO_UNIQUE_ID
#include "hal/unique_id.h"
#import <Foundation/Foundation.h>
#import <IOKit/IOKitLib.h>

static char unique_id_buffer[256] = {0};

bool hal_unique_id_available(void) {
    return true;
}

const char* hal_unique_id_get(void) {
    io_registry_entry_t platformExpert = IOServiceGetMatchingService(
        kIOMasterPortDefault,
        IOServiceMatching("IOPlatformExpertDevice")
    );
    
    if (!platformExpert)
        return NULL;
    
    CFStringRef serialNumber = (CFStringRef)IORegistryEntryCreateCFProperty(
        platformExpert,
        CFSTR("IOPlatformSerialNumber"),
        kCFAllocatorDefault,
        0
    );
    
    IOObjectRelease(platformExpert);
    
    if (!serialNumber)
        return NULL;
    
    if (CFStringGetCString(serialNumber, unique_id_buffer, sizeof(unique_id_buffer), kCFStringEncodingUTF8)) {
        CFRelease(serialNumber);
        return unique_id_buffer;
    }
    
    CFRelease(serialNumber);
    return NULL;
}
#endif // HAL_NO_UNIQUE_ID
