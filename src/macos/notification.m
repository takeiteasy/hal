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

// macOS notifications using UNUserNotificationCenter

#ifndef HAL_NO_NOTIFICATION
#include "hal/notification.h"
#import <Foundation/Foundation.h>
#import <UserNotifications/UserNotifications.h>

bool hal_notification_available(void) {
    return true;
}

bool hal_notification_send(const char *title, const char *message,
                           const char *app_name, int timeout_sec) {
    (void)timeout_sec;
    
    UNMutableNotificationContent *content = [[UNMutableNotificationContent alloc] init];
    
    if (title) {
        content.title = [NSString stringWithUTF8String:title];
    }
    if (message) {
        content.body = [NSString stringWithUTF8String:message];
    }
    if (app_name) {
        content.subtitle = [NSString stringWithUTF8String:app_name];
    }
    content.sound = [UNNotificationSound defaultSound];
    
    NSString *identifier = [[NSUUID UUID] UUIDString];
    UNNotificationRequest *request = [UNNotificationRequest requestWithIdentifier:identifier
                                                                           content:content
                                                                           trigger:nil];
    
    UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
    
    // Request authorization if needed
    [center requestAuthorizationWithOptions:(UNAuthorizationOptionAlert | UNAuthorizationOptionSound)
                          completionHandler:^(BOOL granted, NSError *error) {
        if (granted) {
            [center addNotificationRequest:request withCompletionHandler:nil];
        }
    }];
    
    return true;
}

#endif // HAL_NO_NOTIFICATION
