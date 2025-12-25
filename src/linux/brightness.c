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

// Linux brightness via /sys/class/backlight
// May require appropriate permissions or running as root

#ifndef HAL_NO_BRIGHTNESS
#include "hal/brightness.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#define BACKLIGHT_PATH "/sys/class/backlight"

static char backlight_path[512] = {0};

static bool find_backlight(void) {
    if (backlight_path[0] != '\0')
        return true;
    
    DIR *dir = opendir(BACKLIGHT_PATH);
    if (!dir)
        return false;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;
        
        // Take the first backlight device found
        snprintf(backlight_path, sizeof(backlight_path), "%s/%s", BACKLIGHT_PATH, entry->d_name);
        closedir(dir);
        return true;
    }
    
    closedir(dir);
    return false;
}

static int read_int_file(const char *filename) {
    if (!find_backlight())
        return -1;
    
    char path[768];
    snprintf(path, sizeof(path), "%s/%s", backlight_path, filename);
    
    FILE *f = fopen(path, "r");
    if (!f)
        return -1;
    
    int value = -1;
    if (fscanf(f, "%d", &value) != 1)
        value = -1;
    
    fclose(f);
    return value;
}

static bool write_int_file(const char *filename, int value) {
    if (!find_backlight())
        return false;
    
    char path[768];
    snprintf(path, sizeof(path), "%s/%s", backlight_path, filename);
    
    FILE *f = fopen(path, "w");
    if (!f)
        return false;
    
    int written = fprintf(f, "%d", value);
    fclose(f);
    
    return written > 0;
}

bool hal_brightness_available(void) {
    return find_backlight();
}

float hal_brightness_get(void) {
    int current = read_int_file("brightness");
    int max = read_int_file("max_brightness");
    
    if (current < 0 || max <= 0)
        return -1.0f;
    
    return (float)current / (float)max;
}

bool hal_brightness_set(float level) {
    if (level < 0.0f) level = 0.0f;
    if (level > 1.0f) level = 1.0f;
    
    int max = read_int_file("max_brightness");
    if (max <= 0)
        return false;
    
    int new_value = (int)(level * (float)max);
    return write_int_file("brightness", new_value);
}

#endif // HAL_NO_BRIGHTNESS
