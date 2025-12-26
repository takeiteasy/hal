# hal

> [!WARNING]
> Work in progress

**H**ardware **A**bstraction **L**ibrary -- inspired by [kivy/plyer](https://github.com/kivy/plyer)

## Platform support

| Platform                       | Android | iOS | Windows | macOS | Linux | Web |
| ------------------------------ |:-------:|:---:|:-------:|:-----:|:-----:|:---:|
| Accelerometer                  | YES     | YES | NO      | YES   | YES   | NO  |
| Audio recording                |         |     |         |       |       |     |
| Barometer                      | YES     | YES | NO      | NO    | NO    | NO  |
| Battery                        | YES     | YES | YES     | YES   | YES   | YES |
| Bluetooth                      |         |     |         |       |       |     |
| Brightness                     | YES     | YES | YES     | YES   | YES   | NO  |
| Call                           |         |     |         |       |       |     |
| Camera (taking picture)        |         |     |         |       |       |     |
| Compass                        | YES     | YES | NO      | NO    | NO    | NO  |
| Clipboard                      |         |     |         |       |       |     |
| CPU count                      | YES     | YES | YES     | YES   | YES   | YES |
| Device name                    | YES     | YES | YES     | YES   | YES   | NO  |
| Email (open mail client)       |         |     |         |       |       |     |
| Environment                    | YES     | NO  | YES     | YES   | YES   | NO  |
| File Chooser                   |         |     |         |       |       |     |
| Filesystem                     | YES     | YES | YES     | YES   | YES   | YES |
| Flash                          | YES     | YES | NO      | NO    | NO    | NO  |
| GPS                            |         |     |         |       |       |     |
| Gravity                        | YES     | YES | NO      | NO    | NO    | NO  |
| Gyroscope                      | YES     | YES | NO      | NO    | NO    | NO  |
| Humidity                       | YES     | NO  | NO      | NO    | NO    | NO  |
| IR Blaster                     |         |     |         |       |       |     |
| Keystore                       |         |     |         |       |       |     |
| Light                          | YES     | NO  | NO      | NO    | NO    | NO  |
| Maps                           |         |     |         |       |       |     |
| Notifications                  |         |     |         |       |       |     |
| Orientation                    |         |     |         |       |       |     |
| Proximity                      | YES     | YES | NO      | NO    | NO    | NO  |
| Path Utils                     | YES     | YES | YES     | YES   | YES   | YES |
| Screenshot                     |         |     |         |       |       |     |
| SMS (send messages)            | YES     | YES | YES     | NO    | NO    | NO  |
| Spatial Orientation            | YES     | YES | NO      | NO    | NO    | NO  |
| Shell                          | NO      | NO  | YES     | YES   | YES   | NO  |
| Storage Path                   | YES     | YES | YES     | YES   | YES   | NO  |
| Speech to Text                 |         |     |         |       |       |     |
| Temperature                    | YES     | NO  | NO      | NO    | NO    | NO  |
| Text to Speech                 |         |     |         |       |       |     |
| Threads                        | YES     | YES | YES     | YES   | YES   | [YES](https://emscripten.org/docs/porting/pthreads.html) |
| Unique ID                      | YES     | YES | YES     | YES   | YES   | NO  |
| Vibrator                       | YES     | YES | NO      | NO    | NO    | NO  |
| Voip                           |         |     |         |       |       |     |
| Wifi                           |         |     |         |       |       |     |

## LICENSE

```
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
along with this program.  If not, see <https://www.gnu.org/licenses/>.
```
