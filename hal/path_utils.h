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
 @header path_utils.h
 @copyright George Watson GPLv3
 @updated 2025-12-25
 @brief Path string manipulation utilities (platform-independent)
*/

#ifndef HAL_PATH_UTILS_HEAD
#define HAL_PATH_UTILS_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_PATH_UTILS
#include "hal.h"
#include <stddef.h>

/*!
 @function hal_path_extension
 @param path Path string
 @return Returns pointer to extension (within path) or NULL
 @brief Get file extension from path
*/
const char* hal_path_extension(const char *path);

/*!
 @function hal_path_filename
 @param path Path string
 @return Returns pointer to filename (within path) or NULL
 @brief Get filename from path
*/
const char* hal_path_filename(const char *path);

/*!
 @function hal_path_basename
 @param path Path string
 @return Returns filename without extension (must be freed by caller)
 @brief Get filename without extension
*/
char* hal_path_basename(const char *path);

/*!
 @function hal_path_dirname
 @param path Path string
 @return Returns directory part of path (must be freed by caller)
 @brief Get directory part of path
*/
char* hal_path_dirname(const char *path);

/*!
 @function hal_path_parent
 @param path Path string
 @return Returns parent directory (must be freed by caller)
 @brief Get parent directory of path
*/
char* hal_path_parent(const char *path);

/*!
 @function hal_path_join
 @param a First path component
 @param b Second path component
 @return Returns joined path (must be freed by caller)
 @brief Join two path components
*/
char* hal_path_join(const char *a, const char *b);

/*!
 @function hal_path_normalize
 @param path Path to normalize
 @return Returns normalized path (must be freed by caller)
 @brief Normalize path (resolve . and ..)
*/
char* hal_path_normalize(const char *path);

/*!
 @function hal_path_is_absolute
 @param path Path to check
 @return Returns true if path is absolute
 @brief Check if path is absolute
*/
bool hal_path_is_absolute(const char *path);

/*!
 @function hal_path_split
 @param path Path to split
 @param count Pointer to receive component count
 @return Returns array of path components (must free array and elements)
 @brief Split path into components
*/
char** hal_path_split(const char *path, size_t *count);

/*!
 @function hal_path_free_split
 @param parts Array from hal_path_split
 @param count Number of elements
 @brief Free array from hal_path_split
*/
void hal_path_free_split(char **parts, size_t count);

#ifdef __cplusplus
}
#endif
#endif // HAL_PATH_UTILS_HEAD
