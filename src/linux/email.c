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

// Linux email using xdg-email or mailto: URL

#ifndef HAL_NO_EMAIL
#include "hal/email.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool command_exists(const char *cmd) {
    char buf[256];
    snprintf(buf, sizeof(buf), "command -v %s >/dev/null 2>&1", cmd);
    return system(buf) == 0;
}

static char *shell_escape(const char *str) {
    if (str == NULL) return strdup("");
    
    size_t len = strlen(str);
    char *escaped = malloc(len * 2 + 3);
    char *p = escaped;
    *p++ = '\'';
    
    for (size_t i = 0; i < len; i++) {
        if (str[i] == '\'') {
            *p++ = '\'';
            *p++ = '\\';
            *p++ = '\'';
            *p++ = '\'';
        } else {
            *p++ = str[i];
        }
    }
    *p++ = '\'';
    *p = '\0';
    return escaped;
}

bool hal_email_available(void) {
    return command_exists("xdg-email") || command_exists("xdg-open");
}

bool hal_email_send(const char *recipient, const char *subject,
                    const char *body, const char *cc, const char *bcc) {
    char cmd[4096];
    
    if (command_exists("xdg-email")) {
        snprintf(cmd, sizeof(cmd), "xdg-email");
        
        if (recipient) {
            char *esc = shell_escape(recipient);
            char tmp[512];
            snprintf(tmp, sizeof(tmp), " %s", esc);
            strncat(cmd, tmp, sizeof(cmd) - strlen(cmd) - 1);
            free(esc);
        }
        if (subject) {
            char *esc = shell_escape(subject);
            char tmp[512];
            snprintf(tmp, sizeof(tmp), " --subject %s", esc);
            strncat(cmd, tmp, sizeof(cmd) - strlen(cmd) - 1);
            free(esc);
        }
        if (body) {
            char *esc = shell_escape(body);
            char tmp[1024];
            snprintf(tmp, sizeof(tmp), " --body %s", esc);
            strncat(cmd, tmp, sizeof(cmd) - strlen(cmd) - 1);
            free(esc);
        }
        if (cc) {
            char *esc = shell_escape(cc);
            char tmp[512];
            snprintf(tmp, sizeof(tmp), " --cc %s", esc);
            strncat(cmd, tmp, sizeof(cmd) - strlen(cmd) - 1);
            free(esc);
        }
        if (bcc) {
            char *esc = shell_escape(bcc);
            char tmp[512];
            snprintf(tmp, sizeof(tmp), " --bcc %s", esc);
            strncat(cmd, tmp, sizeof(cmd) - strlen(cmd) - 1);
            free(esc);
        }
        
        strncat(cmd, " &", sizeof(cmd) - strlen(cmd) - 1);
    } else {
        // Fallback to xdg-open with mailto:
        snprintf(cmd, sizeof(cmd), "xdg-open 'mailto:");
        if (recipient)
            strncat(cmd, recipient, sizeof(cmd) - strlen(cmd) - 1);
        strncat(cmd, "' &", sizeof(cmd) - strlen(cmd) - 1);
    }
    
    return system(cmd) == 0;
}

#endif // HAL_NO_EMAIL
