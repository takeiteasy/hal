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

#ifndef HAL_NO_BAROMETER
#include "hal/barometer.h"

bool hal_barometer_available(void) {
    return false;
}

void hal_barometer_enable(void) {}

void hal_barometer_disable(void) {}

bool hal_barometer_enabled(void) {
    return false;
}

bool hal_barometer_disabled(void) {
    return true;
}

bool hal_barometer_toggle(void) {
    return false;
}

bool hal_barometer_pressure(float *pressure) {
    if (pressure)
        *pressure = 0.0f;
    return false;
}
#endif // HAL_NO_BAROMETER
