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

#ifndef HAL_MAPS_HEAD
#define HAL_MAPS_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_MAPS
#include "hal.h"

/*!
 @function hal_maps_available
 @return Returns true if maps functionality is available
 @brief Check if maps can be opened
*/
bool hal_maps_available(void);
/*!
 @function hal_maps_open_address
 @param address Street address to display
 @return Returns true if maps was opened successfully
 @brief Open maps at the specified address
*/
bool hal_maps_open_address(const char *address);
/*!
 @function hal_maps_open_coordinates
 @param lat Latitude in degrees
 @param lon Longitude in degrees
 @param label Optional label for the location (may be NULL)
 @return Returns true if maps was opened successfully
 @brief Open maps at the specified coordinates
*/
bool hal_maps_open_coordinates(double lat, double lon, const char *label);
/*!
 @function hal_maps_search
 @param query Search query
 @param lat Optional center latitude (use 0 to ignore)
 @param lon Optional center longitude (use 0 to ignore)
 @return Returns true if search was opened successfully
 @brief Search for locations in maps
*/
bool hal_maps_search(const char *query, double lat, double lon);
/*!
 @function hal_maps_route
 @param from Starting location (address or coordinates)
 @param to Destination location (address or coordinates)
 @return Returns true if route was displayed successfully
 @brief Get directions from one location to another
*/
bool hal_maps_route(const char *from, const char *to);

#ifdef __cplusplus
}
#endif
#endif // HAL_MAPS_HEAD
