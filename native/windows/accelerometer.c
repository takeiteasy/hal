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

#ifndef HAL_NO_ACCELEROMETER
#include "../accelerometer.h"

bool hal_accelerometer_available(void) {
    return false;
}

void hal_accelerometer_enable(void) {}

void hal_accelerometer_disable(void) {}

bool hal_accelerometer_enabled(void) {
    return false;
}

bool hal_accelerometer_disabled(void) {
    return true;
}

bool hal_accelerometer_toggle(void) {
    return false;
}

bool hal_accelerometer_acceleration(float *x, float *y, float *z) {
    if (x)
        *x = -1;
    if (y)
        *y = -1;
    if (z)
        *z = -1;
    return false
}
#endif // HAL_NO_ACCELEROMETER
