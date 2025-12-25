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

#ifndef HAL_NO_COMPASS
#include "hal/compass.h"

bool hal_compass_available(void) { return false; }
void hal_compass_enable(void) {}
void hal_compass_disable(void) {}
bool hal_compass_enabled(void) { return false; }
bool hal_compass_get(float *x, float *y, float *z) {
    if (x) *x = 0;
    if (y) *y = 0;
    if (z) *z = 0;
    return false;
}
#endif
