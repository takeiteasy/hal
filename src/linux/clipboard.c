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

// Linux clipboard using external tools (wl-copy/wl-paste for Wayland, xclip/xsel for X11)
// This approach avoids library dependencies and works across distributions

#ifndef HAL_NO_CLIPBOARD
#include "hal/clipboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef enum {
    CLIPBOARD_BACKEND_NONE = 0,
    CLIPBOARD_BACKEND_WAYLAND,
    CLIPBOARD_BACKEND_X11_XCLIP,
    CLIPBOARD_BACKEND_X11_XSEL
} clipboard_backend_t;

static clipboard_backend_t detected_backend = CLIPBOARD_BACKEND_NONE;
static bool backend_detected = false;

static bool command_exists(const char *cmd) {
    char buf[256];
    snprintf(buf, sizeof(buf), "command -v %s >/dev/null 2>&1", cmd);
    return system(buf) == 0;
}

static clipboard_backend_t detect_backend(void) {
    if (backend_detected)
        return detected_backend;
    
    backend_detected = true;
    
    // Check for Wayland first
    if (getenv("WAYLAND_DISPLAY") != NULL) {
        if (command_exists("wl-copy") && command_exists("wl-paste")) {
            detected_backend = CLIPBOARD_BACKEND_WAYLAND;
            return detected_backend;
        }
    }
    
    // Check for X11
    if (getenv("DISPLAY") != NULL) {
        if (command_exists("xclip")) {
            detected_backend = CLIPBOARD_BACKEND_X11_XCLIP;
            return detected_backend;
        }
        if (command_exists("xsel")) {
            detected_backend = CLIPBOARD_BACKEND_X11_XSEL;
            return detected_backend;
        }
    }
    
    detected_backend = CLIPBOARD_BACKEND_NONE;
    return detected_backend;
}

static char *run_command_get_output(const char *cmd) {
    FILE *fp = popen(cmd, "r");
    if (fp == NULL)
        return NULL;
    
    size_t capacity = 1024;
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
    
    buffer[size] = '\0';
    pclose(fp);
    
    if (size == 0) {
        free(buffer);
        return NULL;
    }
    
    return buffer;
}

static bool run_command_with_input(const char *cmd, const char *input) {
    FILE *fp = popen(cmd, "w");
    if (fp == NULL)
        return false;
    
    size_t len = strlen(input);
    size_t written = fwrite(input, 1, len, fp);
    int result = pclose(fp);
    
    return written == len && result == 0;
}

bool hal_clipboard_available(void) {
    return detect_backend() != CLIPBOARD_BACKEND_NONE;
}

bool hal_clipboard_has_text(void) {
    char *text = hal_clipboard_get_text();
    if (text == NULL)
        return false;
    
    bool has_text = strlen(text) > 0;
    free(text);
    return has_text;
}

char *hal_clipboard_get_text(void) {
    clipboard_backend_t backend = detect_backend();
    
    switch (backend) {
        case CLIPBOARD_BACKEND_WAYLAND:
            return run_command_get_output("wl-paste --no-newline 2>/dev/null");
        
        case CLIPBOARD_BACKEND_X11_XCLIP:
            return run_command_get_output("xclip -selection clipboard -o 2>/dev/null");
        
        case CLIPBOARD_BACKEND_X11_XSEL:
            return run_command_get_output("xsel --clipboard --output 2>/dev/null");
        
        default:
            return NULL;
    }
}

bool hal_clipboard_set_text(const char *text) {
    if (text == NULL)
        return false;
    
    clipboard_backend_t backend = detect_backend();
    
    switch (backend) {
        case CLIPBOARD_BACKEND_WAYLAND:
            return run_command_with_input("wl-copy", text);
        
        case CLIPBOARD_BACKEND_X11_XCLIP:
            return run_command_with_input("xclip -selection clipboard", text);
        
        case CLIPBOARD_BACKEND_X11_XSEL:
            return run_command_with_input("xsel --clipboard --input", text);
        
        default:
            return false;
    }
}

void hal_clipboard_clear(void) {
    clipboard_backend_t backend = detect_backend();
    
    switch (backend) {
        case CLIPBOARD_BACKEND_WAYLAND:
            system("wl-copy --clear 2>/dev/null");
            break;
        
        case CLIPBOARD_BACKEND_X11_XCLIP:
            system("echo -n | xclip -selection clipboard 2>/dev/null");
            break;
        
        case CLIPBOARD_BACKEND_X11_XSEL:
            system("xsel --clipboard --clear 2>/dev/null");
            break;
        
        default:
            break;
    }
}

#endif // HAL_NO_CLIPBOARD
