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

// Linux file chooser using zenity, kdialog, or yad (runtime detection)

#ifndef HAL_NO_FILE_CHOOSER
#include "hal/file_chooser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef enum {
    DIALOG_BACKEND_NONE = 0,
    DIALOG_BACKEND_ZENITY,
    DIALOG_BACKEND_KDIALOG,
    DIALOG_BACKEND_YAD
} dialog_backend_t;

static dialog_backend_t detected_backend = DIALOG_BACKEND_NONE;
static bool backend_detected = false;

static bool command_exists(const char *cmd) {
    char buf[256];
    snprintf(buf, sizeof(buf), "command -v %s >/dev/null 2>&1", cmd);
    return system(buf) == 0;
}

static dialog_backend_t detect_backend(void) {
    if (backend_detected)
        return detected_backend;
    
    backend_detected = true;
    
    // Check for display availability
    if (getenv("DISPLAY") == NULL && getenv("WAYLAND_DISPLAY") == NULL) {
        detected_backend = DIALOG_BACKEND_NONE;
        return detected_backend;
    }
    
    // Prefer zenity (most common), then kdialog, then yad
    if (command_exists("zenity")) {
        detected_backend = DIALOG_BACKEND_ZENITY;
    } else if (command_exists("kdialog")) {
        detected_backend = DIALOG_BACKEND_KDIALOG;
    } else if (command_exists("yad")) {
        detected_backend = DIALOG_BACKEND_YAD;
    } else {
        detected_backend = DIALOG_BACKEND_NONE;
    }
    
    return detected_backend;
}

static char *run_command_get_output(const char *cmd) {
    FILE *fp = popen(cmd, "r");
    if (fp == NULL)
        return NULL;
    
    size_t capacity = 4096;
    size_t size = 0;
    char *buffer = malloc(capacity);
    if (buffer == NULL) {
        pclose(fp);
        return NULL;
    }
    
    char chunk[256];
    while (fgets(chunk, sizeof(chunk), fp) != NULL) {
        size_t chunk_len = strlen(chunk);
        if (size + chunk_len + 1 > capacity) {
            capacity *= 2;
            char *new_buffer = realloc(buffer, capacity);
            if (new_buffer == NULL) {
                free(buffer);
                pclose(fp);
                return NULL;
            }
            buffer = new_buffer;
        }
        memcpy(buffer + size, chunk, chunk_len);
        size += chunk_len;
    }
    
    int status = pclose(fp);
    
    // Check for cancel (non-zero exit)
    if (status != 0) {
        free(buffer);
        return NULL;
    }
    
    buffer[size] = '\0';
    
    // Trim trailing newline
    if (size > 0 && buffer[size - 1] == '\n') {
        buffer[size - 1] = '\0';
    }
    
    if (strlen(buffer) == 0) {
        free(buffer);
        return NULL;
    }
    
    return buffer;
}

static char *escape_string(const char *str) {
    if (str == NULL) return strdup("");
    
    size_t len = strlen(str);
    char *escaped = malloc(len * 2 + 1);
    char *p = escaped;
    
    for (size_t i = 0; i < len; i++) {
        if (str[i] == '\'' || str[i] == '"' || str[i] == '\\') {
            *p++ = '\\';
        }
        *p++ = str[i];
    }
    *p = '\0';
    
    return escaped;
}

bool hal_file_chooser_available(void) {
    return detect_backend() != DIALOG_BACKEND_NONE;
}

hal_file_chooser_result_t *hal_file_chooser_show(const hal_file_chooser_options_t *options) {
    if (options == NULL)
        return NULL;
    
    dialog_backend_t backend = detect_backend();
    if (backend == DIALOG_BACKEND_NONE)
        return NULL;
    
    char cmd[4096];
    char *title = escape_string(options->title);
    
    switch (backend) {
        case DIALOG_BACKEND_ZENITY:
            if (options->mode == HAL_FILE_CHOOSER_SAVE) {
                snprintf(cmd, sizeof(cmd), "zenity --file-selection --save --confirm-overwrite%s%s%s%s 2>/dev/null",
                         options->title ? " --title='" : "", title,
                         options->title ? "'" : "",
                         options->default_name ? " --filename='" : "");
                if (options->default_name) {
                    char *name = escape_string(options->default_name);
                    strncat(cmd, name, sizeof(cmd) - strlen(cmd) - 1);
                    strncat(cmd, "'", sizeof(cmd) - strlen(cmd) - 1);
                    free(name);
                }
            } else if (options->mode == HAL_FILE_CHOOSER_OPEN_DIRECTORY) {
                snprintf(cmd, sizeof(cmd), "zenity --file-selection --directory%s%s%s 2>/dev/null",
                         options->title ? " --title='" : "", title,
                         options->title ? "'" : "");
            } else {
                snprintf(cmd, sizeof(cmd), "zenity --file-selection%s%s%s%s 2>/dev/null",
                         options->allow_multiple ? " --multiple --separator='|'" : "",
                         options->title ? " --title='" : "", title,
                         options->title ? "'" : "");
            }
            break;
            
        case DIALOG_BACKEND_KDIALOG:
            if (options->mode == HAL_FILE_CHOOSER_SAVE) {
                snprintf(cmd, sizeof(cmd), "kdialog --getsavefilename '%s'%s%s%s 2>/dev/null",
                         options->default_path ? options->default_path : ".",
                         options->title ? " --title '" : "", title,
                         options->title ? "'" : "");
            } else if (options->mode == HAL_FILE_CHOOSER_OPEN_DIRECTORY) {
                snprintf(cmd, sizeof(cmd), "kdialog --getexistingdirectory '%s'%s%s%s 2>/dev/null",
                         options->default_path ? options->default_path : ".",
                         options->title ? " --title '" : "", title,
                         options->title ? "'" : "");
            } else {
                if (options->allow_multiple) {
                    snprintf(cmd, sizeof(cmd), "kdialog --getopenfilename '%s' --multiple --separate-output%s%s%s 2>/dev/null",
                             options->default_path ? options->default_path : ".",
                             options->title ? " --title '" : "", title,
                             options->title ? "'" : "");
                } else {
                    snprintf(cmd, sizeof(cmd), "kdialog --getopenfilename '%s'%s%s%s 2>/dev/null",
                             options->default_path ? options->default_path : ".",
                             options->title ? " --title '" : "", title,
                             options->title ? "'" : "");
                }
            }
            break;
            
        case DIALOG_BACKEND_YAD:
            if (options->mode == HAL_FILE_CHOOSER_SAVE) {
                snprintf(cmd, sizeof(cmd), "yad --file --save --confirm-overwrite%s%s%s 2>/dev/null",
                         options->title ? " --title='" : "", title,
                         options->title ? "'" : "");
            } else if (options->mode == HAL_FILE_CHOOSER_OPEN_DIRECTORY) {
                snprintf(cmd, sizeof(cmd), "yad --file --directory%s%s%s 2>/dev/null",
                         options->title ? " --title='" : "", title,
                         options->title ? "'" : "");
            } else {
                snprintf(cmd, sizeof(cmd), "yad --file%s%s%s%s 2>/dev/null",
                         options->allow_multiple ? " --multiple --separator='|'" : "",
                         options->title ? " --title='" : "", title,
                         options->title ? "'" : "");
            }
            break;
            
        default:
            free(title);
            return NULL;
    }
    
    free(title);
    
    char *output = run_command_get_output(cmd);
    if (output == NULL)
        return NULL;
    
    hal_file_chooser_result_t *result = malloc(sizeof(hal_file_chooser_result_t));
    
    // Parse output - could be single path or multiple separated by | or newline
    char separator = (backend == DIALOG_BACKEND_KDIALOG) ? '\n' : '|';
    
    // Count paths
    int count = 1;
    for (char *p = output; *p; p++) {
        if (*p == separator) count++;
    }
    
    result->paths = malloc(sizeof(char *) * (count + 1));
    result->count = 0;
    
    char *saveptr;
    char sep_str[2] = { separator, '\0' };
    char *token = strtok_r(output, sep_str, &saveptr);
    
    while (token != NULL) {
        // Trim whitespace
        while (*token == ' ' || *token == '\t') token++;
        char *end = token + strlen(token) - 1;
        while (end > token && (*end == ' ' || *end == '\t' || *end == '\n')) *end-- = '\0';
        
        if (strlen(token) > 0) {
            result->paths[result->count++] = strdup(token);
        }
        token = strtok_r(NULL, sep_str, &saveptr);
    }
    
    result->paths[result->count] = NULL;
    free(output);
    
    if (result->count == 0) {
        free(result->paths);
        free(result);
        return NULL;
    }
    
    return result;
}

void hal_file_chooser_result_free(hal_file_chooser_result_t *result) {
    if (result == NULL)
        return;
    
    if (result->paths != NULL) {
        for (int i = 0; i < result->count; i++) {
            free(result->paths[i]);
        }
        free(result->paths);
    }
    free(result);
}

int hal_alert_show(hal_alert_type_t type, const char *title, const char *message,
                   const char **buttons, int button_count) {
    dialog_backend_t backend = detect_backend();
    if (backend == DIALOG_BACKEND_NONE)
        return -1;
    
    char cmd[4096];
    char *etitle = escape_string(title);
    char *emsg = escape_string(message);
    
    const char *icon = "";
    switch (type) {
        case HAL_ALERT_INFO: icon = "info"; break;
        case HAL_ALERT_WARNING: icon = "warning"; break;
        case HAL_ALERT_ERROR: icon = "error"; break;
        case HAL_ALERT_QUESTION: icon = "question"; break;
    }
    
    int result = -1;
    
    switch (backend) {
        case DIALOG_BACKEND_ZENITY:
            if (type == HAL_ALERT_QUESTION && button_count >= 2) {
                snprintf(cmd, sizeof(cmd), "zenity --question --title='%s' --text='%s' 2>/dev/null",
                         etitle, emsg);
                result = (system(cmd) == 0) ? 0 : 1;
            } else {
                snprintf(cmd, sizeof(cmd), "zenity --%s --title='%s' --text='%s' 2>/dev/null",
                         icon, etitle, emsg);
                result = (system(cmd) == 0) ? 0 : -1;
            }
            break;
            
        case DIALOG_BACKEND_KDIALOG:
            if (type == HAL_ALERT_QUESTION && button_count >= 2) {
                snprintf(cmd, sizeof(cmd), "kdialog --yesno '%s' --title '%s' 2>/dev/null",
                         emsg, etitle);
                result = (system(cmd) == 0) ? 0 : 1;
            } else if (type == HAL_ALERT_ERROR) {
                snprintf(cmd, sizeof(cmd), "kdialog --error '%s' --title '%s' 2>/dev/null",
                         emsg, etitle);
                result = (system(cmd) == 0) ? 0 : -1;
            } else {
                snprintf(cmd, sizeof(cmd), "kdialog --msgbox '%s' --title '%s' 2>/dev/null",
                         emsg, etitle);
                result = (system(cmd) == 0) ? 0 : -1;
            }
            break;
            
        case DIALOG_BACKEND_YAD:
            if (type == HAL_ALERT_QUESTION && button_count >= 2) {
                snprintf(cmd, sizeof(cmd), "yad --question --title='%s' --text='%s' 2>/dev/null",
                         etitle, emsg);
                result = (system(cmd) == 0) ? 0 : 1;
            } else {
                snprintf(cmd, sizeof(cmd), "yad --%s --title='%s' --text='%s' --button=OK:0 2>/dev/null",
                         icon, etitle, emsg);
                result = (system(cmd) == 0) ? 0 : -1;
            }
            break;
            
        default:
            break;
    }
    
    free(etitle);
    free(emsg);
    
    return result;
}

#endif // HAL_NO_FILE_CHOOSER
