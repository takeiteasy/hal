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

// Windows unique ID using MachineGuid from registry

#ifndef HAL_NO_UNIQUE_ID
#include "hal/unique_id.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static char unique_id_buffer[256] = {0};

bool hal_unique_id_available(void) {
    return true;
}

const char* hal_unique_id_get(void) {
    HKEY hKey;
    DWORD size = sizeof(unique_id_buffer);
    
    if (RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Cryptography",
        0,
        KEY_READ | KEY_WOW64_64KEY,
        &hKey) != ERROR_SUCCESS) {
        return NULL;
    }
    
    if (RegQueryValueExA(
        hKey,
        "MachineGuid",
        NULL,
        NULL,
        (LPBYTE)unique_id_buffer,
        &size) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return NULL;
    }
    
    RegCloseKey(hKey);
    return unique_id_buffer;
}
#endif // HAL_NO_UNIQUE_ID
