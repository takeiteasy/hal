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

#ifndef HAL_ACCELEROMETER_HEAD
#define HAL_ACCELEROMETER_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_ACCELEROMETER
#include "hal.h"

/*!
 @function hal_accelerometer_available
 @return Returns true if accelerometer is available
 @brief Check if accelerometer is available
*/
bool hal_accelerometer_available(void);
/*!
 @function hal_accelerometer_enable
 @brief Enable accelerometer
*/
void hal_accelerometer_enable(void);
/*!
 @function hal_accelerometer_disable
 @brief Disable accelerometer
*/
void hal_accelerometer_disable(void);
/*!
 @function hal_accelerometer_enabled
 @return Returns true if accelerometer is enabled
 @brief Check if accelerometer is enabled
*/
bool hal_accelerometer_enabled(void);
/*!
 @function hal_accelerometer_disabled
 @return Returns true if accelerometer is disabled
 @brief Check if accelerometer is disabled
*/
bool hal_accelerometer_disabled(void);
/*!
 @function hal_accelerometer_toggle
 @return Returns the new state of the accelerometer
 @brief Toggle accelerometer state
*/
bool hal_accelerometer_toggle(void);
/*!
 @function hal_accelerometer_acceleration
 @param x Pointer to store x acceleration
 @param y Pointer to store y acceleration
 @param z Pointer to store z acceleration
 @return Returns true on success
 @brief Get current acceleration
*/
bool hal_accelerometer_acceleration(float *x, float *y, float *z);

#ifdef __cplusplus
}
#endif
#endif // HAL_ACCELEROMETER_HEAD
