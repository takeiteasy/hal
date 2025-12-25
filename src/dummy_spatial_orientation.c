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

#ifndef HAL_NO_SPATIAL_ORIENTATION
#include "hal/spatial_orientation.h"

bool hal_spatial_orientation_available(void) { return false; }
void hal_spatial_orientation_enable(void) {}
void hal_spatial_orientation_disable(void) {}
bool hal_spatial_orientation_enabled(void) { return false; }
bool hal_spatial_orientation_get(float *yaw, float *pitch, float *roll) {
    if (yaw) *yaw = 0;
    if (pitch) *pitch = 0;
    if (roll) *roll = 0;
    return false;
}
#endif
