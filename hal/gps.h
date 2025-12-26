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

#ifndef HAL_GPS_HEAD
#define HAL_GPS_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_GPS
#include "hal.h"

/*!
 @typedef hal_gps_location_cb
 @param lat Latitude in degrees
 @param lon Longitude in degrees
 @param altitude Altitude in meters above sea level
 @param speed Speed in meters per second
 @param bearing Bearing in degrees
 @param ctx User context pointer
 @discussion Callback for GPS location updates
*/
typedef void (*hal_gps_location_cb)(double lat, double lon, double altitude,
                                     double speed, double bearing, void *ctx);
/*!
 @typedef hal_gps_status_cb
 @param status Status message
 @param ctx User context pointer
 @discussion Callback for GPS status changes
*/
typedef void (*hal_gps_status_cb)(const char *status, void *ctx);

/*!
 @function hal_gps_available
 @return Returns true if GPS is available
 @brief Check if GPS functionality is available
*/
bool hal_gps_available(void);
/*!
 @function hal_gps_configure
 @param on_location Callback for location updates
 @param on_status Callback for status changes (may be NULL)
 @param ctx User context pointer passed to callbacks
 @brief Configure GPS callbacks. Must be called before hal_gps_start.
*/
void hal_gps_configure(hal_gps_location_cb on_location, hal_gps_status_cb on_status, void *ctx);
/*!
 @function hal_gps_start
 @param min_time_ms Minimum time between updates in milliseconds
 @param min_distance_m Minimum distance between updates in meters
 @return Returns true if GPS was started successfully
 @brief Start receiving GPS location updates
*/
bool hal_gps_start(int min_time_ms, float min_distance_m);
/*!
 @function hal_gps_stop
 @brief Stop receiving GPS location updates
*/
void hal_gps_stop(void);

#ifdef __cplusplus
}
#endif
#endif // HAL_GPS_HEAD
