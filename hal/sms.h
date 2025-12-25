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

/*!
 @header sms.h
 @copyright George Watson GPLv3
 @updated 2025-12-25
 @brief SMS Messaging API
*/

#ifndef HAL_SMS_HEAD
#define HAL_SMS_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ONLY_SMS
#include "hal.h"

/*!
 @function hal_sms_available
 @return Returns true if SMS sending is available
 @brief Check if SMS sending is available
*/
bool hal_sms_available(void);
/*!
 @function hal_sms_send
 @param recipient Phone number or contact
 @param message Message text to send
 @return Returns true on success
 @brief Send an SMS message
*/
bool hal_sms_send(const char *recipient, const char *message);

#ifdef __cplusplus
}
#endif
#endif // HAL_SMS_HEAD
