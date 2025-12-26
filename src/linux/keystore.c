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

// Linux keystore using secret-tool (libsecret CLI)

#ifndef HAL_NO_KEYSTORE
#include "hal/keystore.h"
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

bool hal_keystore_available(void) {
    return command_exists("secret-tool");
}

bool hal_keystore_set(const char *service, const char *key, const char *value) {
    if (!service || !key || !value) return false;
    
    char *esc_service = shell_escape(service);
    char *esc_key = shell_escape(key);
    char *esc_value = shell_escape(value);
    
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), 
             "echo -n %s | secret-tool store --label=%s service %s key %s 2>/dev/null",
             esc_value, esc_key, esc_service, esc_key);
    
    free(esc_service);
    free(esc_key);
    free(esc_value);
    
    return system(cmd) == 0;
}

char *hal_keystore_get(const char *service, const char *key) {
    if (!service || !key) return NULL;
    
    char *esc_service = shell_escape(service);
    char *esc_key = shell_escape(key);
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "secret-tool lookup service %s key %s 2>/dev/null",
             esc_service, esc_key);
    
    free(esc_service);
    free(esc_key);
    
    FILE *fp = popen(cmd, "r");
    if (!fp) return NULL;
    
    char buf[4096];
    char *result = NULL;
    if (fgets(buf, sizeof(buf), fp)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
        result = strdup(buf);
    }
    
    pclose(fp);
    return result;
}

bool hal_keystore_delete(const char *service, const char *key) {
    if (!service || !key) return false;
    
    char *esc_service = shell_escape(service);
    char *esc_key = shell_escape(key);
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "secret-tool clear service %s key %s 2>/dev/null",
             esc_service, esc_key);
    
    free(esc_service);
    free(esc_key);
    
    return system(cmd) == 0;
}

#endif // HAL_NO_KEYSTORE
