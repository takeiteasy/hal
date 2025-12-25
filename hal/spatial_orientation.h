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
 @header spatial_orientation.h
 @copyright George Watson GPLv3
 @updated 2025-12-25
 @brief Spatial Orientation (Device Attitude) API
*/

#ifndef HAL_SPATIAL_ORIENTATION_HEAD
#define HAL_SPATIAL_ORIENTATION_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_SPATIAL_ORIENTATION
#include "hal.h"

/*!
 @function hal_spatial_orientation_available
 @return Returns true if spatial orientation is available
 @brief Check if spatial orientation is available
*/
bool hal_spatial_orientation_available(void);
/*!
 @function hal_spatial_orientation_enable
 @brief Enable spatial orientation sensing
*/
void hal_spatial_orientation_enable(void);
/*!
 @function hal_spatial_orientation_disable
 @brief Disable spatial orientation sensing
*/
void hal_spatial_orientation_disable(void);
/*!
 @function hal_spatial_orientation_enabled
 @return Returns true if spatial orientation is enabled
 @brief Check if spatial orientation is enabled
*/
bool hal_spatial_orientation_enabled(void);
/*!
 @function hal_spatial_orientation_get
 @param yaw Pointer to store yaw (azimuth) in radians
 @param pitch Pointer to store pitch in radians
 @param roll Pointer to store roll in radians
 @return Returns true on success
 @brief Get device spatial orientation (attitude)
*/
bool hal_spatial_orientation_get(float *yaw, float *pitch, float *roll);

#ifdef __cplusplus
}
#endif
#endif // HAL_SPATIAL_ORIENTATION_HEAD
