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

// Linux notifications using notify-send

#ifndef HAL_NO_NOTIFICATION
#include "hal/notification.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool command_exists(const char *cmd) {
    char buf[256];
    snprintf(buf, sizeof(buf), "command -v %s >/dev/null 2>&1", cmd);
    return system(buf) == 0;
}

static char *shell_escape(const char *str) {
    if (!str) return strdup("");
    size_t len = strlen(str);
    char *escaped = malloc(len * 4 + 3);
    char *p = escaped;
    *p++ = '\'';
    for (size_t i = 0; i < len; i++) {
        if (str[i] == '\'') {
            *p++ = '\''; *p++ = '\\'; *p++ = '\''; *p++ = '\'';
        } else {
            *p++ = str[i];
        }
    }
    *p++ = '\'';
    *p = '\0';
    return escaped;
}

bool hal_notification_available(void) {
    return command_exists("notify-send");
}

bool hal_notification_send(const char *title, const char *message,
                           const char *app_name, int timeout_sec) {
    char cmd[2048] = "notify-send";
    
    if (app_name) {
        char *esc = shell_escape(app_name);
        char tmp[256];
        snprintf(tmp, sizeof(tmp), " -a %s", esc);
        strncat(cmd, tmp, sizeof(cmd) - strlen(cmd) - 1);
        free(esc);
    }
    
    if (timeout_sec > 0) {
        char tmp[64];
        snprintf(tmp, sizeof(tmp), " -t %d", timeout_sec * 1000);
        strncat(cmd, tmp, sizeof(cmd) - strlen(cmd) - 1);
    }
    
    if (title) {
        char *esc = shell_escape(title);
        strncat(cmd, " ", sizeof(cmd) - strlen(cmd) - 1);
        strncat(cmd, esc, sizeof(cmd) - strlen(cmd) - 1);
        free(esc);
    } else {
        strncat(cmd, " ''", sizeof(cmd) - strlen(cmd) - 1);
    }
    
    if (message) {
        char *esc = shell_escape(message);
        strncat(cmd, " ", sizeof(cmd) - strlen(cmd) - 1);
        strncat(cmd, esc, sizeof(cmd) - strlen(cmd) - 1);
        free(esc);
    }
    
    return system(cmd) == 0;
}

#endif // HAL_NO_NOTIFICATION
