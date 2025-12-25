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

#ifndef HAL_GYROSCOPE_HEAD
#define HAL_GYROSCOPE_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_GYROSCOPE
#include "hal.h"

/*!
 @function hal_gyroscope_available
 @return Returns true if gyroscope is available
 @brief Check if gyroscope is available
*/
bool hal_gyroscope_available(void);
/*!
 @function hal_gyroscope_enable
 @brief Enable gyroscope
*/
void hal_gyroscope_enable(void);
/*!
 @function hal_gyroscope_disable
 @brief Disable gyroscope
*/
void hal_gyroscope_disable(void);
/*!
 @function hal_gyroscope_enabled
 @return Returns true if gyroscope is enabled
 @brief Check if gyroscope is enabled
*/
bool hal_gyroscope_enabled(void);
/*!
 @function hal_gyroscope_get
 @param x Pointer to store x rotation
 @param y Pointer to store y rotation
 @param z Pointer to store z rotation
 @return Returns true on success
 @brief Get rotation rate (x, y, z) in radians per second
*/
bool hal_gyroscope_get(float *x, float *y, float *z);

#ifdef __cplusplus
}
#endif
#endif // HAL_GYROSCOPE_HEAD