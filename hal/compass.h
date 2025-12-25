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

#ifndef HAL_COMPASS_HEAD
#define HAL_COMPASS_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_COMPASS
#include "hal.h"

/*!
 @function hal_compass_available
 @return Returns true if compass is available
 @brief Check if compass is available
*/
bool hal_compass_available(void);
/*!
 @function hal_compass_enable
 @brief Enable compass
*/
void hal_compass_enable(void);
/*!
 @function hal_compass_disable
 @brief Disable compass
*/
void hal_compass_disable(void);
/*!
 @function hal_compass_enabled
 @return Returns true if compass is enabled
 @brief Check if compass is enabled
*/
bool hal_compass_enabled(void);
/*!
 @function hal_compass_get
 @param x Pointer to store x magnetic field
 @param y Pointer to store y magnetic field
 @param z Pointer to store z magnetic field
 @return Returns true on success
 @brief Get magnetic field values (x, y, z) in microtesla
*/
bool hal_compass_get(float *x, float *y, float *z);

#ifdef __cplusplus
}
#endif
#endif // HAL_COMPASS_HEAD