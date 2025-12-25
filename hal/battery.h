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

#ifndef HAL_BATTERY_HEAD
#define HAL_BATTERY_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_BATTERY
#include "hal.h"

/*!
 @enum hal_battery_status_t
 @constant HAL_BATTERY_STATUS_UNKNOWN Unknown status
 @constant HAL_BATTERY_STATUS_CHARGING Charging
 @constant HAL_BATTERY_STATUS_DISCHARGING Discharging
 @constant HAL_BATTERY_STATUS_FULL Battery full
 @constant HAL_BATTERY_STATUS_NOT_CHARGING Not charging
 @constant HAL_BATTERY_STATUS_NO_BATTERY No battery
 @discussion Battery status enum
*/
typedef enum {
    HAL_BATTERY_STATUS_UNKNOWN = 0,
    HAL_BATTERY_STATUS_CHARGING,
    HAL_BATTERY_STATUS_DISCHARGING,
    HAL_BATTERY_STATUS_FULL,
    HAL_BATTERY_STATUS_NOT_CHARGING,
    HAL_BATTERY_STATUS_NO_BATTERY
} hal_battery_status_t;

/*!
 @function hal_battery_available
 @return Returns true if battery is available
 @brief Check if battery is available
*/
bool hal_battery_available(void);
/*!
 @function hal_battery_level
 @return Returns battery level (0-100), -1 on error
 @brief Get battery level
*/
int hal_battery_level(void);
/*!
 @function hal_battery_status
 @return Returns battery status
 @brief Get battery status
*/
hal_battery_status_t hal_battery_status(void);
/*!
 @function hal_battery_is_charging
 @return Returns true if battery is charging
 @brief Check if battery is charging
*/
bool hal_battery_is_charging(void);
/*!
 @function hal_battery_is_plugged
 @return Returns true if battery is plugged in
 @brief Check if battery is plugged in
*/
bool hal_battery_is_plugged(void);

#ifdef __cplusplus
}
#endif
#endif // HAL_BATTERY_HEAD