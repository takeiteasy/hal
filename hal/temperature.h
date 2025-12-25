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

#ifndef HAL_TEMPERATURE_HEAD
#define HAL_TEMPERATURE_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_TEMPERATURE
#include "hal.h"

/*!
 @function hal_temperature_available
 @return Returns true if temperature sensor is available
 @brief Check if ambient temperature sensor is available
*/
bool hal_temperature_available(void);
/*!
 @function hal_temperature_enable
 @brief Enable temperature sensor
*/
void hal_temperature_enable(void);
/*!
 @function hal_temperature_disable
 @brief Disable temperature sensor
*/
void hal_temperature_disable(void);
/*!
 @function hal_temperature_enabled
 @return Returns true if temperature sensor is enabled
 @brief Check if temperature sensor is enabled
*/
bool hal_temperature_enabled(void);
/*!
 @function hal_temperature_get
 @return Returns ambient temperature in Celsius, -999.0f on failure
 @brief Get ambient temperature
*/
float hal_temperature_get(void);

#ifdef __cplusplus
}
#endif
#endif // HAL_TEMPERATURE_HEAD