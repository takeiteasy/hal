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

// macOS storagepath using NSSearchPathForDirectoriesInDomains

#ifndef HAL_NO_STORAGEPATH
#include "hal/storagepath.h"
#import <Foundation/Foundation.h>

static char home_buf[1024];
static char documents_buf[1024];
static char downloads_buf[1024];
static char desktop_buf[1024];
static char pictures_buf[1024];
static char music_buf[1024];  
static char videos_buf[1024];
static char appdata_buf[1024];
static char cache_buf[1024];
static char temp_buf[1024];

static const char* get_ns_path(NSSearchPathDirectory dir, char *buf) {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(dir, NSUserDomainMask, YES);
    if ([paths count] > 0) {
        const char *path = [[paths objectAtIndex:0] UTF8String];
        strncpy(buf, path, 1023);
        buf[1023] = '\0';
        return buf;
    }
    return NULL;
}

bool hal_storagepath_available(void) { return true; }

const char* hal_path_home(void) {
    const char *home = [NSHomeDirectory() UTF8String];
    if (home) {
        strncpy(home_buf, home, 1023);
        home_buf[1023] = '\0';
        return home_buf;
    }
    return NULL;
}

const char* hal_path_documents(void) {
    return get_ns_path(NSDocumentDirectory, documents_buf);
}

const char* hal_path_downloads(void) {
    return get_ns_path(NSDownloadsDirectory, downloads_buf);
}

const char* hal_path_desktop(void) {
    return get_ns_path(NSDesktopDirectory, desktop_buf);
}

const char* hal_path_pictures(void) {
    return get_ns_path(NSPicturesDirectory, pictures_buf);
}

const char* hal_path_music(void) {
    return get_ns_path(NSMusicDirectory, music_buf);
}

const char* hal_path_videos(void) {
    return get_ns_path(NSMoviesDirectory, videos_buf);
}

const char* hal_path_appdata(void) {
    return get_ns_path(NSApplicationSupportDirectory, appdata_buf);
}

const char* hal_path_cache(void) {
    return get_ns_path(NSCachesDirectory, cache_buf);
}

const char* hal_path_temp(void) {
    const char *tmp = [NSTemporaryDirectory() UTF8String];
    if (tmp) {
        strncpy(temp_buf, tmp, 1023);
        temp_buf[1023] = '\0';
        return temp_buf;
    }
    return "/tmp";
}
#endif
