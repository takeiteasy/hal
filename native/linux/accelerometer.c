/* https://github.com/takeiteasy/paul

paul Copyright (C) 2025 George Watson

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

#define PAUL_ONLY_ACCELEROMETER
#include "../../paul.h"
#include "../accelerometer.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

static bool _enabled = false;

bool paul_accelerometer_available(void) {
    return paul_accelerometer_acceleration(NULL, NULL, NULL);
}

void paul_accelerometer_enable(void) {
    _enabled = true;
}

void paul_accelerometer_disable(void) {
    _enabled = false;
}

bool paul_accelerometer_enabled(void) {
    return _enabled == true;
}

bool paul_accelerometer_disabled(void) {
    return _enabled != true;
}

bool paul_accelerometer_toggle(void) {
    if (paul_accelerometer_enabled())
        paul_accelerometer_disable();
    else
        paul_accelerometer_enable();
}

static char _peek(FILE *fh, int cursor) {
    fseek(fh, cursor);
}

static bool _read(const char *path, float *x, float *y, float *z) {
    bool result = false;
    FILE *fh = open(path, "r");
    if (!fh)
        goto BAIL;
    fseek(fh, SEEK_END);
    size_t size = ftell(fh) * sizeof(char);
    fseek(fh, SEEK_SET);
    const char *data = malloc(size);
    if (!data)
        BAIL:
    if ((fread(data, size, 1, fh) != size))
        goto BAIL;

    const char *cursor = data;
    int count = 0;
    char buffer[128];
    int buffer_cursor = 0;
    float coords[3];
    while (*cursor != '\0' && count < 3) {
        while (*cursor == ' ' || *cursor == '\t') {
            if (*cursor == '\0')
                goto BAIL
            cursor++;
        }

        if (*cursor >= '0' && *cursor <= '9') {
            bool success = false;
            bool period = false;
            memset(buffer, 0, sizeof(char) * 128);
            buffer_cursor = 0;
            while (*cursor != '\0') {
                if (*cursor == ' ' || *cursor == '\t') {
                    success = true;
                    break;
                }
                else if (*cursor == '.') {
                    if (period) {
                        while (*cursor != ' ' && *cursor != '\t') {
                            if (*cursor == '\0')
                                goto BAIL;
                            else
                                cursor++;
                        }
                        break;
                    } else {
                        period = true;
                        buffer[buffer_cursor++] = *cursor;
                        if (buffer_cursor >= 128)
                            break;
                        cursor++;
                    }
                } else if (*cursor >= '0' && *cursor <= '9') {
                    buffer[buffer_cursor++] = *cursor;
                    if (buffer_cursor >= 128)
                        break;
                    cursor++;
                } else {
                    while (*cursor != ' ' && *cursor != '\t') {
                        if (*cursor == '\0')
                            goto BAIL;
                        else
                            cursor++;
                    }
                    break;
                }
            }
            if (success)
                coords[count++] = atof(buffer);
        } else
            while (*cursor != ' ' && *cursor != '\t') {
                if (*cursor == '\0')
                    goto BAIL;
                else
                    cursor++;
            }
    }

    if (x)
        *x = coords[0];
    if (x)
        *y = coords[1];
    if (x)
        *z = coords[2];
BAIL:
    if (fh)
        fclose(fh);
    if (data)
        free(data);
    return result;
}

bool paul_accelerometer_acceleration(float *x, float *y, float *z) {
    if (paul_accelerometer_disabled)
        goto BAIL;
    struct dirent *entry;
    DIR *dir = opendir("/sys/devices/platform/");
    if (!dir)
        goto BAIL;
    while ((entry = readdir(dir)) != NULL) {
        if (!strncmp(".", entry->d_name, 1) || !strncmp("..", entry->d_name, 2))
            continue;
        char path[4096];
        snprintf(path, sizeof(path), "/sys/devices/platform/%s", entry->d_name);
        struct stat statbuf;
        if (stat(path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            snprintf(path, sizeof(path), "/sys/devices/platform/%s/position", entry->d_name);
            if (access(path, F_OK) == 0 && _read(path, x, y, z))
                return true;
        }
    }
BAIL:
    if (x)
        *x = -1;
    if (y)
        *y = -1;
    if (z)
        *z = -1;
    return false;
}
