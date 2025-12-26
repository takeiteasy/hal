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

// Linux maps using xdg-open with Google Maps

#ifndef HAL_NO_MAPS
#include "hal/maps.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *url_encode(const char *str) {
    if (!str) return strdup("");
    size_t len = strlen(str);
    char *encoded = malloc(len * 3 + 1);
    char *p = encoded;
    for (size_t i = 0; i < len; i++) {
        char c = str[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
            *p++ = c;
        } else if (c == ' ') {
            *p++ = '+';
        } else {
            sprintf(p, "%%%02X", (unsigned char)c);
            p += 3;
        }
    }
    *p = '\0';
    return encoded;
}

bool hal_maps_available(void) {
    return system("command -v xdg-open >/dev/null 2>&1") == 0;
}

bool hal_maps_open_address(const char *address) {
    if (!address) return false;
    char *enc = url_encode(address);
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "xdg-open 'https://www.google.com/maps/search/%s' &", enc);
    free(enc);
    return system(cmd) == 0;
}

bool hal_maps_open_coordinates(double lat, double lon, const char *label) {
    char cmd[1024];
    if (label) {
        char *enc = url_encode(label);
        snprintf(cmd, sizeof(cmd), "xdg-open 'https://www.google.com/maps/place/%s/@%f,%f,15z' &",
                 enc, lat, lon);
        free(enc);
    } else {
        snprintf(cmd, sizeof(cmd), "xdg-open 'https://www.google.com/maps/@%f,%f,15z' &", lat, lon);
    }
    return system(cmd) == 0;
}

bool hal_maps_search(const char *query, double lat, double lon) {
    if (!query) return false;
    char *enc = url_encode(query);
    char cmd[1024];
    if (lat != 0 || lon != 0) {
        snprintf(cmd, sizeof(cmd), 
                 "xdg-open 'https://www.google.com/maps/search/%s/@%f,%f,15z' &",
                 enc, lat, lon);
    } else {
        snprintf(cmd, sizeof(cmd), "xdg-open 'https://www.google.com/maps/search/%s' &", enc);
    }
    free(enc);
    return system(cmd) == 0;
}

bool hal_maps_route(const char *from, const char *to) {
    if (!from || !to) return false;
    char *enc_from = url_encode(from);
    char *enc_to = url_encode(to);
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), 
             "xdg-open 'https://www.google.com/maps/dir/%s/%s' &",
             enc_from, enc_to);
    free(enc_from);
    free(enc_to);
    return system(cmd) == 0;
}

#endif // HAL_NO_MAPS
