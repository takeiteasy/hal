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

#ifndef HAL_NO_IR_BLASTER
#include "hal/ir_blaster.h"

bool hal_ir_available(void) {
    return false;
}

bool hal_ir_transmit(int frequency, const int *pattern, int pattern_len) {
    (void)frequency; (void)pattern; (void)pattern_len;
    return false;
}

int hal_ir_get_frequencies(int *min_freq, int *max_freq, int max_ranges) {
    (void)min_freq; (void)max_freq; (void)max_ranges;
    return 0;
}

#endif // HAL_NO_IR_BLASTER
