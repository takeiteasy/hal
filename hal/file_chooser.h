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

#ifndef HAL_FILE_CHOOSER_HEAD
#define HAL_FILE_CHOOSER_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_FILE_CHOOSER
#include "hal.h"

/*!
 @enum hal_file_chooser_mode_t
 @constant HAL_FILE_CHOOSER_OPEN Open one or more files
 @constant HAL_FILE_CHOOSER_SAVE Save a file
 @constant HAL_FILE_CHOOSER_OPEN_DIRECTORY Select a directory
 @discussion File chooser dialog mode
*/
typedef enum {
    HAL_FILE_CHOOSER_OPEN = 0,
    HAL_FILE_CHOOSER_SAVE,
    HAL_FILE_CHOOSER_OPEN_DIRECTORY
} hal_file_chooser_mode_t;

/*!
 @struct hal_file_chooser_options_t
 @field mode Dialog mode (open, save, or directory)
 @field title Dialog title (NULL for default)
 @field default_path Initial directory path (NULL for default)
 @field default_name Default filename for save dialogs (NULL for none)
 @field filters NULL-terminated array of filter patterns (e.g. "*.txt", "*.jpg;*.png")
 @field filter_names NULL-terminated array of filter display names (e.g. "Text Files")
 @field allow_multiple Allow multiple file selection (open mode only)
 @discussion Options for configuring file chooser dialogs
*/
typedef struct {
    hal_file_chooser_mode_t mode;
    const char *title;
    const char *default_path;
    const char *default_name;
    const char **filters;
    const char **filter_names;
    bool allow_multiple;
} hal_file_chooser_options_t;

/*!
 @struct hal_file_chooser_result_t
 @field paths NULL-terminated array of selected file paths
 @field count Number of paths in the array
 @discussion Result from a file chooser dialog
*/
typedef struct {
    char **paths;
    int count;
} hal_file_chooser_result_t;

/*!
 @enum hal_alert_type_t
 @constant HAL_ALERT_INFO Informational message
 @constant HAL_ALERT_WARNING Warning message
 @constant HAL_ALERT_ERROR Error message
 @constant HAL_ALERT_QUESTION Question dialog
 @discussion Alert dialog type
*/
typedef enum {
    HAL_ALERT_INFO = 0,
    HAL_ALERT_WARNING,
    HAL_ALERT_ERROR,
    HAL_ALERT_QUESTION
} hal_alert_type_t;

/*!
 @function hal_file_chooser_available
 @return Returns true if file chooser functionality is available
 @brief Check if file chooser is available on this platform
*/
bool hal_file_chooser_available(void);
/*!
 @function hal_file_chooser_show
 @param options Pointer to options structure configuring the dialog
 @return Returns pointer to result structure, or NULL on cancel/error. Caller must free with hal_file_chooser_result_free.
 @brief Show a file chooser dialog
*/
hal_file_chooser_result_t *hal_file_chooser_show(const hal_file_chooser_options_t *options);
/*!
 @function hal_file_chooser_result_free
 @param result Pointer to result structure to free
 @brief Free a file chooser result structure
*/
void hal_file_chooser_result_free(hal_file_chooser_result_t *result);

/*!
 @function hal_alert_show
 @param type Alert type (info, warning, error, question)
 @param title Dialog title
 @param message Dialog message
 @param buttons NULL-terminated array of button labels
 @param button_count Number of buttons
 @return Returns 0-based index of clicked button, or -1 on cancel/close
 @brief Show an alert dialog
*/
int hal_alert_show(hal_alert_type_t type, const char *title, const char *message,
                   const char **buttons, int button_count);

#ifdef __cplusplus
}
#endif
#endif // HAL_FILE_CHOOSER_HEAD
