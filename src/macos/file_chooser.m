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

#ifndef HAL_NO_FILE_CHOOSER
#include "hal/file_chooser.h"
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

bool hal_file_chooser_available(void) {
    return true;
}

static NSArray<UTType *> *filters_to_content_types(const char **filters) {
    if (filters == NULL || filters[0] == NULL)
        return @[UTTypeItem];
    
    NSMutableArray<UTType *> *types = [NSMutableArray array];
    
    for (int i = 0; filters[i] != NULL; i++) {
        NSString *filter = [NSString stringWithUTF8String:filters[i]];
        // Parse semicolon-separated patterns like "*.jpg;*.png"
        NSArray<NSString *> *patterns = [filter componentsSeparatedByString:@";"];
        
        for (NSString *pattern in patterns) {
            NSString *ext = [pattern stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
            // Remove leading "*."
            if ([ext hasPrefix:@"*."]) {
                ext = [ext substringFromIndex:2];
            } else if ([ext hasPrefix:@"."]) {
                ext = [ext substringFromIndex:1];
            }
            
            UTType *type = [UTType typeWithFilenameExtension:ext];
            if (type != nil) {
                [types addObject:type];
            }
        }
    }
    
    return types.count > 0 ? types : @[UTTypeItem];
}

hal_file_chooser_result_t *hal_file_chooser_show(const hal_file_chooser_options_t *options) {
    if (options == NULL)
        return NULL;
    
    __block hal_file_chooser_result_t *result = NULL;
    
    // Must run on main thread for UI
    dispatch_block_t showDialog = ^{
        if (options->mode == HAL_FILE_CHOOSER_SAVE) {
            NSSavePanel *panel = [NSSavePanel savePanel];
            
            if (options->title)
                panel.title = [NSString stringWithUTF8String:options->title];
            if (options->default_path)
                panel.directoryURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:options->default_path]];
            if (options->default_name)
                panel.nameFieldStringValue = [NSString stringWithUTF8String:options->default_name];
            
            panel.allowedContentTypes = filters_to_content_types(options->filters);
            
            if ([panel runModal] == NSModalResponseOK && panel.URL != nil) {
                result = malloc(sizeof(hal_file_chooser_result_t));
                result->count = 1;
                result->paths = malloc(sizeof(char *) * 2);
                result->paths[0] = strdup(panel.URL.path.UTF8String);
                result->paths[1] = NULL;
            }
        } else {
            NSOpenPanel *panel = [NSOpenPanel openPanel];
            
            if (options->title)
                panel.title = [NSString stringWithUTF8String:options->title];
            if (options->default_path)
                panel.directoryURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:options->default_path]];
            
            if (options->mode == HAL_FILE_CHOOSER_OPEN_DIRECTORY) {
                panel.canChooseFiles = NO;
                panel.canChooseDirectories = YES;
            } else {
                panel.canChooseFiles = YES;
                panel.canChooseDirectories = NO;
                panel.allowedContentTypes = filters_to_content_types(options->filters);
            }
            
            panel.allowsMultipleSelection = options->allow_multiple;
            
            if ([panel runModal] == NSModalResponseOK && panel.URLs.count > 0) {
                result = malloc(sizeof(hal_file_chooser_result_t));
                result->count = (int)panel.URLs.count;
                result->paths = malloc(sizeof(char *) * (result->count + 1));
                
                for (int i = 0; i < result->count; i++) {
                    result->paths[i] = strdup(panel.URLs[i].path.UTF8String);
                }
                result->paths[result->count] = NULL;
            }
        }
    };
    
    if ([NSThread isMainThread]) {
        showDialog();
    } else {
        dispatch_sync(dispatch_get_main_queue(), showDialog);
    }
    
    return result;
}

void hal_file_chooser_result_free(hal_file_chooser_result_t *result) {
    if (result == NULL)
        return;
    
    if (result->paths != NULL) {
        for (int i = 0; i < result->count; i++) {
            free(result->paths[i]);
        }
        free(result->paths);
    }
    free(result);
}

int hal_alert_show(hal_alert_type_t type, const char *title, const char *message,
                   const char **buttons, int button_count) {
    __block int clicked = -1;
    
    dispatch_block_t showAlert = ^{
        NSAlert *alert = [[NSAlert alloc] init];
        
        if (title)
            alert.messageText = [NSString stringWithUTF8String:title];
        if (message)
            alert.informativeText = [NSString stringWithUTF8String:message];
        
        switch (type) {
            case HAL_ALERT_INFO:
                alert.alertStyle = NSAlertStyleInformational;
                break;
            case HAL_ALERT_WARNING:
                alert.alertStyle = NSAlertStyleWarning;
                break;
            case HAL_ALERT_ERROR:
                alert.alertStyle = NSAlertStyleCritical;
                break;
            case HAL_ALERT_QUESTION:
                alert.alertStyle = NSAlertStyleInformational;
                break;
        }
        
        // Add buttons (first button is default)
        for (int i = 0; i < button_count && buttons && buttons[i]; i++) {
            [alert addButtonWithTitle:[NSString stringWithUTF8String:buttons[i]]];
        }
        
        if (button_count == 0 || buttons == NULL) {
            [alert addButtonWithTitle:@"OK"];
        }
        
        NSModalResponse response = [alert runModal];
        clicked = (int)(response - NSAlertFirstButtonReturn);
    };
    
    if ([NSThread isMainThread]) {
        showAlert();
    } else {
        dispatch_sync(dispatch_get_main_queue(), showAlert);
    }
    
    return clicked;
}

#endif // HAL_NO_FILE_CHOOSER
