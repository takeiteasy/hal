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

#ifndef HAL_LIGHT_HEAD
#define HAL_LIGHT_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_LIGHT
#include "hal.h"

/*!
 @function hal_light_available
 @return Returns true if light sensor is available
 @brief Check if ambient light sensor is available
*/
bool hal_light_available(void);
/*!
 @function hal_light_enable
 @brief Enable light sensor
*/
void hal_light_enable(void);
/*!
 @function hal_light_disable
 @brief Disable light sensor
*/
void hal_light_disable(void);
/*!
 @function hal_light_enabled
 @return Returns true if light sensor is enabled
 @brief Check if light sensor is enabled
*/
bool hal_light_enabled(void);
/*!
 @function hal_light_get
 @return Returns illumination in lux, -1.0f on failure
 @brief Get illumination
*/
float hal_light_get(void);

#ifdef __cplusplus
}
#endif
#endif // HAL_LIGHT_HEAD