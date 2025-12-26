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

// iOS call using tel: URL scheme

#ifndef HAL_NO_CALL
#include "hal/call.h"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

bool hal_call_available(void) {
    NSURL *url = [NSURL URLWithString:@"tel://"];
    return [[UIApplication sharedApplication] canOpenURL:url];
}

bool hal_call_dial(const char *tel) {
    if (tel == NULL) return false;
    
    NSString *urlString = [NSString stringWithFormat:@"telprompt://%s", tel];
    NSURL *url = [NSURL URLWithString:urlString];
    
    if ([[UIApplication sharedApplication] canOpenURL:url]) {
        [[UIApplication sharedApplication] openURL:url options:@{} completionHandler:nil];
        return true;
    }
    return false;
}

bool hal_call_make(const char *tel) {
    if (tel == NULL) return false;
    
    NSString *urlString = [NSString stringWithFormat:@"tel://%s", tel];
    NSURL *url = [NSURL URLWithString:urlString];
    
    if ([[UIApplication sharedApplication] canOpenURL:url]) {
        [[UIApplication sharedApplication] openURL:url options:@{} completionHandler:nil];
        return true;
    }
    return false;
}

#endif // HAL_NO_CALL
