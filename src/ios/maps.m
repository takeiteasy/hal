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

// iOS maps using Apple Maps URL scheme

#ifndef HAL_NO_MAPS
#include "hal/maps.h"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

static NSString *url_encode(const char *str) {
    if (!str) return @"";
    NSString *ns = [NSString stringWithUTF8String:str];
    return [ns stringByAddingPercentEncodingWithAllowedCharacters:
            [NSCharacterSet URLQueryAllowedCharacterSet]];
}

static bool open_url(NSString *urlString) {
    NSURL *url = [NSURL URLWithString:urlString];
    if ([[UIApplication sharedApplication] canOpenURL:url]) {
        [[UIApplication sharedApplication] openURL:url options:@{} completionHandler:nil];
        return true;
    }
    return false;
}

bool hal_maps_available(void) {
    NSURL *url = [NSURL URLWithString:@"http://maps.apple.com/"];
    return [[UIApplication sharedApplication] canOpenURL:url];
}

bool hal_maps_open_address(const char *address) {
    if (!address) return false;
    NSString *urlString = [NSString stringWithFormat:@"http://maps.apple.com/?address=%@",
                           url_encode(address)];
    return open_url(urlString);
}

bool hal_maps_open_coordinates(double lat, double lon, const char *label) {
    NSString *urlString;
    if (label) {
        urlString = [NSString stringWithFormat:@"http://maps.apple.com/?ll=%f,%f&q=%@",
                     lat, lon, url_encode(label)];
    } else {
        urlString = [NSString stringWithFormat:@"http://maps.apple.com/?ll=%f,%f", lat, lon];
    }
    return open_url(urlString);
}

bool hal_maps_search(const char *query, double lat, double lon) {
    if (!query) return false;
    NSString *urlString;
    if (lat != 0 || lon != 0) {
        urlString = [NSString stringWithFormat:@"http://maps.apple.com/?q=%@&sll=%f,%f",
                     url_encode(query), lat, lon];
    } else {
        urlString = [NSString stringWithFormat:@"http://maps.apple.com/?q=%@", url_encode(query)];
    }
    return open_url(urlString);
}

bool hal_maps_route(const char *from, const char *to) {
    if (!from || !to) return false;
    NSString *urlString = [NSString stringWithFormat:@"http://maps.apple.com/?saddr=%@&daddr=%@",
                           url_encode(from), url_encode(to)];
    return open_url(urlString);
}

#endif // HAL_NO_MAPS
