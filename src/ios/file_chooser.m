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

// iOS file chooser using UIDocumentPickerViewController and UIAlertController
// Note: Requires app to set the presenting view controller via hal_file_chooser_ios_set_view_controller

#ifndef HAL_NO_FILE_CHOOSER
#include "hal/file_chooser.h"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

// The app must set this view controller before using file chooser
static UIViewController *g_presenting_vc = nil;
static dispatch_semaphore_t g_semaphore = nil;
static hal_file_chooser_result_t *g_result = nil;
static int g_alert_result = -1;

// App must call this to set the presenting view controller
void hal_file_chooser_ios_set_view_controller(void *vc) {
    g_presenting_vc = (__bridge UIViewController *)vc;
}

@interface HALDocumentPickerDelegate : NSObject <UIDocumentPickerDelegate>
@end

@implementation HALDocumentPickerDelegate

- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
    if (urls.count > 0) {
        g_result = malloc(sizeof(hal_file_chooser_result_t));
        g_result->count = (int)urls.count;
        g_result->paths = malloc(sizeof(char *) * (g_result->count + 1));
        
        for (int i = 0; i < g_result->count; i++) {
            // Start accessing security-scoped resource
            [urls[i] startAccessingSecurityScopedResource];
            g_result->paths[i] = strdup(urls[i].path.UTF8String);
            [urls[i] stopAccessingSecurityScopedResource];
        }
        g_result->paths[g_result->count] = NULL;
    }
    dispatch_semaphore_signal(g_semaphore);
}

- (void)documentPickerWasCancelled:(UIDocumentPickerViewController *)controller {
    g_result = NULL;
    dispatch_semaphore_signal(g_semaphore);
}

@end

static HALDocumentPickerDelegate *g_picker_delegate = nil;

bool hal_file_chooser_available(void) {
    return g_presenting_vc != nil;
}

static NSArray<UTType *> *filters_to_content_types(const char **filters) {
    if (filters == NULL || filters[0] == NULL)
        return @[UTTypeItem];
    
    NSMutableArray<UTType *> *types = [NSMutableArray array];
    
    for (int i = 0; filters[i] != NULL; i++) {
        NSString *filter = [NSString stringWithUTF8String:filters[i]];
        NSArray<NSString *> *patterns = [filter componentsSeparatedByString:@";"];
        
        for (NSString *pattern in patterns) {
            NSString *ext = [pattern stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
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
    if (options == NULL || g_presenting_vc == nil)
        return NULL;
    
    g_result = NULL;
    g_semaphore = dispatch_semaphore_create(0);
    
    if (g_picker_delegate == nil) {
        g_picker_delegate = [[HALDocumentPickerDelegate alloc] init];
    }
    
    dispatch_async(dispatch_get_main_queue(), ^{
        UIDocumentPickerViewController *picker = nil;
        
        if (options->mode == HAL_FILE_CHOOSER_OPEN_DIRECTORY) {
            picker = [[UIDocumentPickerViewController alloc] initForOpeningContentTypes:@[UTTypeFolder]];
        } else if (options->mode == HAL_FILE_CHOOSER_SAVE) {
            // For save, we need a temporary file - this is a simplified implementation
            // In practice, apps should use UIDocumentPickerViewController with forExporting:
            NSArray<UTType *> *types = filters_to_content_types(options->filters);
            picker = [[UIDocumentPickerViewController alloc] initForOpeningContentTypes:types];
        } else {
            NSArray<UTType *> *types = filters_to_content_types(options->filters);
            picker = [[UIDocumentPickerViewController alloc] initForOpeningContentTypes:types];
            picker.allowsMultipleSelection = options->allow_multiple;
        }
        
        picker.delegate = g_picker_delegate;
        
        if (options->default_path) {
            NSURL *dirURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:options->default_path]];
            picker.directoryURL = dirURL;
        }
        
        [g_presenting_vc presentViewController:picker animated:YES completion:nil];
    });
    
    dispatch_semaphore_wait(g_semaphore, DISPATCH_TIME_FOREVER);
    return g_result;
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
    if (g_presenting_vc == nil)
        return -1;
    
    g_alert_result = -1;
    g_semaphore = dispatch_semaphore_create(0);
    
    dispatch_async(dispatch_get_main_queue(), ^{
        UIAlertController *alert = [UIAlertController alertControllerWithTitle:title ? [NSString stringWithUTF8String:title] : nil
                                                                       message:message ? [NSString stringWithUTF8String:message] : nil
                                                                preferredStyle:UIAlertControllerStyleAlert];
        
        for (int i = 0; i < button_count && buttons && buttons[i]; i++) {
            UIAlertActionStyle style = (i == 0) ? UIAlertActionStyleDefault : UIAlertActionStyleDefault;
            int buttonIndex = i;
            
            UIAlertAction *action = [UIAlertAction actionWithTitle:[NSString stringWithUTF8String:buttons[i]]
                                                            style:style
                                                          handler:^(UIAlertAction *action) {
                g_alert_result = buttonIndex;
                dispatch_semaphore_signal(g_semaphore);
            }];
            [alert addAction:action];
        }
        
        if (button_count == 0 || buttons == NULL) {
            UIAlertAction *ok = [UIAlertAction actionWithTitle:@"OK"
                                                        style:UIAlertActionStyleDefault
                                                      handler:^(UIAlertAction *action) {
                g_alert_result = 0;
                dispatch_semaphore_signal(g_semaphore);
            }];
            [alert addAction:ok];
        }
        
        [g_presenting_vc presentViewController:alert animated:YES completion:nil];
    });
    
    dispatch_semaphore_wait(g_semaphore, DISPATCH_TIME_FOREVER);
    return g_alert_result;
}

#endif // HAL_NO_FILE_CHOOSER
