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

#ifndef HAL_IR_BLASTER_HEAD
#define HAL_IR_BLASTER_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_IR_BLASTER
#include "hal.h"

/*!
 @function hal_ir_available
 @return Returns true if IR blaster is available
 @brief Check if IR blaster hardware is available
*/
bool hal_ir_available(void);
/*!
 @function hal_ir_transmit
 @param frequency Carrier frequency in Hz
 @param pattern Array of on/off durations in microseconds
 @param pattern_len Length of the pattern array
 @return Returns true if transmission was successful
 @brief Transmit an IR sequence
*/
bool hal_ir_transmit(int frequency, const int *pattern, int pattern_len);
/*!
 @function hal_ir_get_frequencies
 @param min_freq Array to receive minimum frequencies (may be NULL)
 @param max_freq Array to receive maximum frequencies (may be NULL)
 @param max_ranges Maximum number of frequency ranges to return
 @return Returns number of frequency ranges available
 @brief Get supported IR frequency ranges
*/
int hal_ir_get_frequencies(int *min_freq, int *max_freq, int max_ranges);

#ifdef __cplusplus
}
#endif
#endif // HAL_IR_BLASTER_HEAD
