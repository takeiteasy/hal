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

/*!
 @header proximity.h
 @copyright George Watson GPLv3
 @updated 2025-12-25
 @brief Proximity Sensor API
*/

#ifndef HAL_PROXIMITY_HEAD
#define HAL_PROXIMITY_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_PROXIMITY
#include "hal.h"

/*!
 @function hal_proximity_available
 @return Returns true if proximity sensor is available
 @brief Check if proximity sensor is available
*/
bool hal_proximity_available(void);
/*!
 @function hal_proximity_enable
 @brief Enable proximity sensor
*/
void hal_proximity_enable(void);
/*!
 @function hal_proximity_disable
 @brief Disable proximity sensor
*/
void hal_proximity_disable(void);
/*!
 @function hal_proximity_enabled
 @return Returns true if proximity sensor is enabled
 @brief Check if proximity sensor is enabled
*/
bool hal_proximity_enabled(void);
/*!
 @function hal_proximity_get
 @return Returns proximity distance in cm, -1.0f on failure
 @brief Get proximity sensor value
*/
float hal_proximity_get(void);
/*!
 @function hal_proximity_is_near
 @return Returns true if object is near the sensor
 @brief Check if object is near
*/
bool hal_proximity_is_near(void);

#ifdef __cplusplus
}
#endif
#endif // HAL_PROXIMITY_HEAD
