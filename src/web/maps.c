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

// Web maps using Google Maps URL

#ifndef HAL_NO_MAPS
#include "hal/maps.h"
#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

EM_JS(void, js_open_url, (const char *url), {
    var urlStr = UTF8ToString(url);
    window.open(urlStr, '_blank');
});

bool hal_maps_available(void) {
    return true;
}

bool hal_maps_open_address(const char *address) {
    if (!address) return false;
    char url[1024];
    snprintf(url, sizeof(url), "https://www.google.com/maps/search/%s", address);
    js_open_url(url);
    return true;
}

bool hal_maps_open_coordinates(double lat, double lon, const char *label) {
    char url[512];
    if (label) {
        snprintf(url, sizeof(url), "https://www.google.com/maps/place/%s/@%f,%f,15z",
                 label, lat, lon);
    } else {
        snprintf(url, sizeof(url), "https://www.google.com/maps/@%f,%f,15z", lat, lon);
    }
    js_open_url(url);
    return true;
}

bool hal_maps_search(const char *query, double lat, double lon) {
    if (!query) return false;
    char url[1024];
    if (lat != 0 || lon != 0) {
        snprintf(url, sizeof(url), "https://www.google.com/maps/search/%s/@%f,%f,15z",
                 query, lat, lon);
    } else {
        snprintf(url, sizeof(url), "https://www.google.com/maps/search/%s", query);
    }
    js_open_url(url);
    return true;
}

bool hal_maps_route(const char *from, const char *to) {
    if (!from || !to) return false;
    char url[2048];
    snprintf(url, sizeof(url), "https://www.google.com/maps/dir/%s/%s", from, to);
    js_open_url(url);
    return true;
}

#endif // HAL_NO_MAPS
