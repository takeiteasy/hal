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

// iOS SMS using sms: URL scheme

#ifndef HAL_NO_SMS
#include "hal/sms.h"
#import <UIKit/UIKit.h>
#include <string.h>

bool hal_sms_available(void) {
    return [[UIApplication sharedApplication] canOpenURL:[NSURL URLWithString:@"sms:"]];
}

bool hal_sms_send(const char *recipient, const char *message) {
    if (!recipient)
        return false;
    
    (void)message; // iOS sms: URL doesn't support pre-filling message via URL
    
    NSString *urlString = [NSString stringWithFormat:@"sms:%s", recipient];
    NSURL *url = [NSURL URLWithString:urlString];
    
    if ([[UIApplication sharedApplication] canOpenURL:url]) {
        [[UIApplication sharedApplication] openURL:url options:@{} completionHandler:nil];
        return true;
    }
    return false;
}
#endif
