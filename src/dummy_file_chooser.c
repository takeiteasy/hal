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

#ifndef HAL_NO_FILE_CHOOSER
#include "hal/file_chooser.h"

bool hal_file_chooser_available(void) {
    return false;
}

hal_file_chooser_result_t *hal_file_chooser_show(const hal_file_chooser_options_t *options) {
    (void)options;
    return NULL;
}

void hal_file_chooser_result_free(hal_file_chooser_result_t *result) {
    (void)result;
}

int hal_alert_show(hal_alert_type_t type, const char *title, const char *message,
                   const char **buttons, int button_count) {
    (void)type;
    (void)title;
    (void)message;
    (void)buttons;
    (void)button_count;
    return -1;
}

#endif // HAL_NO_FILE_CHOOSER
