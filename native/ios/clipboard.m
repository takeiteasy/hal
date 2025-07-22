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
#import <UIKit/UIKit.h>

bool paul_clipboard_available(void) {
    return [UIPasteboard generalPasteboard] != NULL;
}

const char *paul_clipboard_get(void) {
    UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
    return strdup([[pasteboard string] UTF8String]);
}

void paul_clipboard_set(const char *str) {
    UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
    [pasteboard setString:[NSString stringWithUTF8String:str]];
}
#endif // PAUL_NO_CLIPBOARD
