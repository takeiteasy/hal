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

// iOS email using mailto: URL

#ifndef HAL_NO_EMAIL
#include "hal/email.h"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

static NSString *url_encode(const char *str) {
    if (str == NULL) return @"";
    NSString *ns = [NSString stringWithUTF8String:str];
    return [ns stringByAddingPercentEncodingWithAllowedCharacters:
            [NSCharacterSet URLQueryAllowedCharacterSet]];
}

bool hal_email_available(void) {
    NSURL *url = [NSURL URLWithString:@"mailto:"];
    return [[UIApplication sharedApplication] canOpenURL:url];
}

bool hal_email_send(const char *recipient, const char *subject,
                    const char *body, const char *cc, const char *bcc) {
    NSMutableString *urlString = [NSMutableString stringWithString:@"mailto:"];
    
    if (recipient)
        [urlString appendString:url_encode(recipient)];
    
    NSMutableArray *params = [NSMutableArray array];
    if (subject)
        [params addObject:[NSString stringWithFormat:@"subject=%@", url_encode(subject)]];
    if (body)
        [params addObject:[NSString stringWithFormat:@"body=%@", url_encode(body)]];
    if (cc)
        [params addObject:[NSString stringWithFormat:@"cc=%@", url_encode(cc)]];
    if (bcc)
        [params addObject:[NSString stringWithFormat:@"bcc=%@", url_encode(bcc)]];
    
    if (params.count > 0) {
        [urlString appendString:@"?"];
        [urlString appendString:[params componentsJoinedByString:@"&"]];
    }
    
    NSURL *url = [NSURL URLWithString:urlString];
    if ([[UIApplication sharedApplication] canOpenURL:url]) {
        [[UIApplication sharedApplication] openURL:url options:@{} completionHandler:nil];
        return true;
    }
    return false;
}

#endif // HAL_NO_EMAIL
