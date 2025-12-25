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

#ifndef HAL_NO_ENVIRONMENT
#include "hal/environment.h"
#include <windows.h>
#include <stdlib.h>

bool hal_environment_available(void) { return true; }

char* hal_environment_get(const char *name) {
    if (!name) return NULL;
    
    DWORD size = GetEnvironmentVariableA(name, NULL, 0);
    if (size == 0) return NULL;
    
    char *value = (char*)malloc(size);
    if (!value) return NULL;
    
    if (GetEnvironmentVariableA(name, value, size) == 0) {
        free(value);
        return NULL;
    }
    return value;
}

bool hal_environment_set(const char *name, const char *value) {
    if (!name) return false;
    return SetEnvironmentVariableA(name, value) != 0;
}
#endif
