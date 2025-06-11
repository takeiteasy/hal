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

#ifndef PAUL_NO_FILE_SYSTEM
#include "../file_system.h"
#include <stdio.h>
#include <unistd.h>
#include "../shared/file_system.c"
#import <Cocoa/Cocoa.h>

bool paul_file_exists(const char *path) {
    return [[NSFileManager defaultManager] fileExistsAtPath:@(path)
                                                isDirectory:nil];
}

bool paul_dir_exists(const char *path) {
    bool result = false;
    [[NSFileManager defaultManager] fileExistsAtPath:@(path)
                                         isDirectory:&result];
    return result;
}

// WARNING: These must be released
const char* paul_working_dir(void) {
    NSString *url = [[NSFileManager defaultManager] currentDirectoryPath];
    const char *result = strdup([url UTF8String]);
    [url release];
    return result;
}

const char* paul_home_dir(void) {
    NSURL *url = [[NSFileManager defaultManager] homeDirectoryForCurrentUser];
    const char *result = strdup([[url absoluteString] UTF8String]);
    [url release];
    return result;
}

const char* paul_get_root_dir(void) {
    return strdup("/");
}

static const char* _storage_path(NSSearchPathDirectory dir) {
    NSURL *url = [[[NSFileManager defaultManager] URLsForDirectory:dir
                                                         inDomains:1] firstObject];
    const char *result = strdup([[url absoluteString] UTF8String]);
    [url release];
    return result;
}

const char* paul_documents_dir(void) {
    return _storage_path(NSDocumentDirectory);
}

const char* paul_downloads_dir(void) {
    return _storage_path(NSDownloadsDirectory);
}

const char* paul_video_dir(void) {
    return _storage_path(NSMoviesDirectory);
}

const char* paul_music_dir(void) {
    return _storage_path(NSMusicDirectory);
}

const char* paul_picture_dir(void) {
    return _storage_path(NSPicturesDirectory);
}

const char* paul_application_dir(void) {
    return _storage_path(NSApplicationDirectory);
}

const char* paul_desktop_dir(void) {
    return _storage_path(NSDesktopDirectory);
}

#endif // PAUL_NO_FILE_SYSTEM
