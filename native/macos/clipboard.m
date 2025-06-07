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

#ifndef PAUL_NO_CLIPBOARD
#include "../clipboard.h"
#import <AppKit/AppKit.h>

bool paul_clipboard_available(void) {
    return [NSPasteboard generalPasteboard] != NULL;
}

const char *paul_clipboard_get(void) {
    NSPasteboard *pb = [NSPasteboard generalPasteboard];
    if ([[pb types] containsObject:NSPasteboardTypeString]) {
        NSString *text = [pb stringForType:NSPasteboardTypeString];
        return strdup([text UTF8String]);
    }
    return NULL;
}

void paul_clipboard_set(const char *str) {
    NSPasteboard *pb = [NSPasteboard generalPasteboard];
    [pb clearContents];
    [pb setString:@(str)
          forType:NSPasteboardTypeString];
}
#endif // PAUL_NO_CLIPBOARD
