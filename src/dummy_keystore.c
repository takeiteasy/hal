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

#ifndef HAL_NO_KEYSTORE
#include "hal/keystore.h"

bool hal_keystore_available(void) { return false; }

bool hal_keystore_set(const char *service, const char *key, const char *value) {
    (void)service; (void)key; (void)value;
    return false;
}

char *hal_keystore_get(const char *service, const char *key) {
    (void)service; (void)key;
    return NULL;
}

bool hal_keystore_delete(const char *service, const char *key) {
    (void)service; (void)key;
    return false;
}

#endif // HAL_NO_KEYSTORE
