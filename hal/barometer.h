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

#ifndef HAL_BAROMETER_HEAD
#define HAL_BAROMETER_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_BAROMETER
#include "hal.h"

/*!
 @function hal_barometer_available
 @return Returns true if barometer is available
 @brief Check if barometer is available
*/
bool hal_barometer_available(void);
/*!
 @function hal_barometer_enable
 @brief Enable barometer
*/
void hal_barometer_enable(void);
/*!
 @function hal_barometer_disable
 @brief Disable barometer
*/
void hal_barometer_disable(void);
/*!
 @function hal_barometer_enabled
 @return Returns true if barometer is enabled
 @brief Check if barometer is enabled
*/
bool hal_barometer_enabled(void);
/*!
 @function hal_barometer_disabled
 @return Returns true if barometer is disabled
 @brief Check if barometer is disabled
*/
bool hal_barometer_disabled(void);
/*!
 @function hal_barometer_toggle
 @return Returns the new state of the barometer
 @brief Toggle barometer state
*/
bool hal_barometer_toggle(void);
/*!
 @function hal_barometer_pressure
 @param pressure Pointer to store pressure
 @return Returns true on success
 @brief Get current pressure
*/
bool hal_barometer_pressure(float *pressure);

#ifdef __cplusplus
}
#endif
#endif // HAL_BAROMETER_HEAD