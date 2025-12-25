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
 @header storagepath.h
 @copyright George Watson GPLv3
 @updated 2025-12-25
 @brief System directory paths (home, documents, downloads, etc.)
*/

#ifndef HAL_STORAGEPATH_HEAD
#define HAL_STORAGEPATH_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_STORAGEPATH
#include "hal.h"

/*!
 @function hal_storagepath_available
 @return Returns true if storage paths are available
 @brief Check if storage path queries are supported
*/
bool hal_storagepath_available(void);

/*!
 @function hal_path_home
 @return Returns home directory path (static buffer)
 @brief Get user's home directory
*/
const char* hal_path_home(void);

/*!
 @function hal_path_documents
 @return Returns documents directory path (static buffer)
 @brief Get user's documents directory
*/
const char* hal_path_documents(void);

/*!
 @function hal_path_downloads
 @return Returns downloads directory path (static buffer)
 @brief Get user's downloads directory
*/
const char* hal_path_downloads(void);

/*!
 @function hal_path_desktop
 @return Returns desktop directory path (static buffer)
 @brief Get user's desktop directory
*/
const char* hal_path_desktop(void);

/*!
 @function hal_path_pictures
 @return Returns pictures directory path (static buffer)
 @brief Get user's pictures directory
*/
const char* hal_path_pictures(void);

/*!
 @function hal_path_music
 @return Returns music directory path (static buffer)
 @brief Get user's music directory
*/
const char* hal_path_music(void);

/*!
 @function hal_path_videos
 @return Returns videos directory path (static buffer)
 @brief Get user's videos directory
*/
const char* hal_path_videos(void);

/*!
 @function hal_path_appdata
 @return Returns application data directory path (static buffer)
 @brief Get application data directory
*/
const char* hal_path_appdata(void);

/*!
 @function hal_path_cache
 @return Returns cache directory path (static buffer)
 @brief Get cache directory
*/
const char* hal_path_cache(void);

/*!
 @function hal_path_temp
 @return Returns temporary directory path (static buffer)
 @brief Get temporary directory
*/
const char* hal_path_temp(void);

#ifdef __cplusplus
}
#endif
#endif // HAL_STORAGEPATH_HEAD
