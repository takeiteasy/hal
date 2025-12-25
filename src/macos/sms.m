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

// macOS SMS using Messages app via AppleScript

#ifndef HAL_NO_SMS
#include "hal/sms.h"
#import <Foundation/Foundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool hal_sms_available(void) {
    // Check if osascript is available
    return access("/usr/bin/osascript", X_OK) == 0;
}

bool hal_sms_send(const char *recipient, const char *message) {
    if (!recipient || !message)
        return false;
    
    // Escape quotes in message and recipient
    NSString *escapedRecipient = [[NSString stringWithUTF8String:recipient]
        stringByReplacingOccurrencesOfString:@"\"" withString:@"\\\""];
    NSString *escapedMessage = [[NSString stringWithUTF8String:message]
        stringByReplacingOccurrencesOfString:@"\"" withString:@"\\\""];
    
    // Build AppleScript
    NSString *script = [NSString stringWithFormat:
        @"tell application \"Messages\"\n"
        @"    set targetService to 1st account whose service type = iMessage\n"
        @"    set targetBuddy to participant \"%@\" of targetService\n"
        @"    send \"%@\" to targetBuddy\n"
        @"end tell", escapedRecipient, escapedMessage];
    
    // Execute via osascript
    NSTask *task = [[NSTask alloc] init];
    [task setLaunchPath:@"/usr/bin/osascript"];
    [task setArguments:@[@"-e", script]];
    
    NSPipe *pipe = [NSPipe pipe];
    [task setStandardOutput:pipe];
    [task setStandardError:pipe];
    
    @try {
        [task launch];
        [task waitUntilExit];
        return [task terminationStatus] == 0;
    } @catch (NSException *e) {
        return false;
    }
}
#endif
