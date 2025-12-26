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

#ifndef HAL_NOTIFICATION_HEAD
#define HAL_NOTIFICATION_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_NOTIFICATION
#include "hal.h"

/*!
 @function hal_notification_available
 @return Returns true if notifications are available
 @brief Check if system notifications are available
*/
bool hal_notification_available(void);
/*!
 @function hal_notification_send
 @param title Notification title
 @param message Notification message body
 @param app_name Application name (may be NULL)
 @param timeout_sec Display timeout in seconds (0 for default)
 @return Returns true if notification was sent successfully
 @brief Send a system notification
*/
bool hal_notification_send(const char *title, const char *message,
                           const char *app_name, int timeout_sec);

#ifdef __cplusplus
}
#endif
#endif // HAL_NOTIFICATION_HEAD
