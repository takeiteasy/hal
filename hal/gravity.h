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

#ifndef HAL_GRAVITY_HEAD
#define HAL_GRAVITY_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_GRAVITY
#include "hal.h"

/*!
 @function hal_gravity_available
 @return Returns true if gravity sensor is available
 @brief Check if gravity sensor is available
*/
bool hal_gravity_available(void);
/*!
 @function hal_gravity_enable
 @brief Enable gravity sensor
*/
void hal_gravity_enable(void);
/*!
 @function hal_gravity_disable
 @brief Disable gravity sensor
*/
void hal_gravity_disable(void);
/*!
 @function hal_gravity_enabled
 @return Returns true if gravity sensor is enabled
 @brief Check if gravity sensor is enabled
*/
bool hal_gravity_enabled(void);
/*!
 @function hal_gravity_get
 @param x Pointer to store x gravity
 @param y Pointer to store y gravity
 @param z Pointer to store z gravity
 @return Returns true on success
 @brief Get gravity vector (x, y, z) in m/s^2
*/
bool hal_gravity_get(float *x, float *y, float *z);

#ifdef __cplusplus
}
#endif
#endif // HAL_GRAVITY_HEAD