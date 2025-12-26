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

#ifndef HAL_NO_CLIPBOARD
#include "hal/clipboard.h"
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

bool hal_clipboard_available(void) {
    return true;
}

bool hal_clipboard_has_text(void) {
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    NSArray *types = [pasteboard types];
    return [types containsObject:NSPasteboardTypeString];
}

char *hal_clipboard_get_text(void) {
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    NSString *text = [pasteboard stringForType:NSPasteboardTypeString];
    if (text == nil)
        return NULL;
    
    const char *utf8 = [text UTF8String];
    if (utf8 == NULL)
        return NULL;
    
    return strdup(utf8);
}

bool hal_clipboard_set_text(const char *text) {
    if (text == NULL)
        return false;
    
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    [pasteboard clearContents];
    NSString *nsText = [NSString stringWithUTF8String:text];
    if (nsText == nil)
        return false;
    
    return [pasteboard setString:nsText forType:NSPasteboardTypeString];
}

void hal_clipboard_clear(void) {
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    [pasteboard clearContents];
}

#endif // HAL_NO_CLIPBOARD
