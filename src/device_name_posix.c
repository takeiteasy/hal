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

// Common POSIX device name implementation using gethostname()
// Used by macOS, Linux, and can be used by other POSIX platforms

#ifndef HAL_NO_DEVICE_NAME
#include "hal/device_name.h"
#include <unistd.h>
#include <string.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

static char hostname_buffer[HOST_NAME_MAX + 1] = {0};

bool hal_device_name_available(void) {
    return true;
}

const char* hal_device_name_get(void) {
    if (gethostname(hostname_buffer, HOST_NAME_MAX) == 0) {
        hostname_buffer[HOST_NAME_MAX] = '\0';
        return hostname_buffer;
    }
    return NULL;
}
#endif // HAL_NO_DEVICE_NAME
