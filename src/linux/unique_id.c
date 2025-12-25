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

// Linux unique ID using /etc/machine-id or /var/lib/dbus/machine-id

#ifndef HAL_NO_UNIQUE_ID
#include "hal/unique_id.h"
#include <stdio.h>
#include <string.h>

static char unique_id_buffer[64] = {0};

bool hal_unique_id_available(void) {
    return hal_unique_id_get() != NULL;
}

const char* hal_unique_id_get(void) {
    FILE *f = fopen("/etc/machine-id", "r");
    if (!f) {
        f = fopen("/var/lib/dbus/machine-id", "r");
    }
    
    if (!f)
        return NULL;
    
    if (fgets(unique_id_buffer, sizeof(unique_id_buffer), f)) {
        // Remove trailing newline
        size_t len = strlen(unique_id_buffer);
        if (len > 0 && unique_id_buffer[len - 1] == '\n') {
            unique_id_buffer[len - 1] = '\0';
        }
        fclose(f);
        return unique_id_buffer;
    }
    
    fclose(f);
    return NULL;
}
#endif // HAL_NO_UNIQUE_ID
