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

#ifndef HAL_NO_GPS
#include "hal/gps.h"

bool hal_gps_available(void) {
    return false;
}

void hal_gps_configure(hal_gps_location_cb on_location, hal_gps_status_cb on_status, void *ctx) {
    (void)on_location; (void)on_status; (void)ctx;
}

bool hal_gps_start(int min_time_ms, float min_distance_m) {
    (void)min_time_ms; (void)min_distance_m;
    return false;
}

void hal_gps_stop(void) {
}

#endif // HAL_NO_GPS
