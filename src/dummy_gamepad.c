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

#ifndef HAL_NO_GAMEPAD
#include "gamepad_common.h"

bool hal_gamepad_available(void) {
    return false;
}

void hal_gamepad_init(void) {
}

void hal_gamepad_shutdown(void) {
}

unsigned int hal_gamepad_num_devices(void) {
    return 0;
}

hal_gamepad_device_t *hal_gamepad_device_at(unsigned int index) {
    (void)index;
    return NULL;
}

void hal_gamepad_detect_devices(void) {
}

void hal_gamepad_process_events(void) {
}

#endif /* HAL_NO_GAMEPAD */
