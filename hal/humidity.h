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

#ifndef HAL_HUMIDITY_HEAD
#define HAL_HUMIDITY_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_HUMIDITY
#include "hal.h"

/*!
 @function hal_humidity_available
 @return Returns true if humidity sensor is available
 @brief Check if relative humidity sensor is available
*/
bool hal_humidity_available(void);
/*!
 @function hal_humidity_enable
 @brief Enable humidity sensor
*/
void hal_humidity_enable(void);
/*!
 @function hal_humidity_disable
 @brief Disable humidity sensor
*/
void hal_humidity_disable(void);
/*!
 @function hal_humidity_enabled
 @return Returns true if humidity sensor is enabled
 @brief Check if humidity sensor is enabled
*/
bool hal_humidity_enabled(void);
/*!
 @function hal_humidity_get
 @return Returns relative humidity in percent, -1.0f on failure
 @brief Get relative humidity
*/
float hal_humidity_get(void);

#ifdef __cplusplus
}
#endif
#endif // HAL_HUMIDITY_HEAD