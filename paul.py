import os

names = [
  "accelerometer",
  "arguments",
  "audio_recording",
  "barometer",
  "battery",
  "bluetooth",
  "brightness",
  "call",
  "camera",
  "compass",
  "clipboard",
  "cpu_count",
  "device_name",
  "email",
  "file_chooser",
  "file_system",
  "flash",
  "gps",
  "gravity",
  "gyroscope",
  "humidity",
  "ir_blaster",
  "keystore",
  "light",
  "maps",
  "notifications",
  "orientation",
  "proximity",
  "screenshot",
  "sms",
  "spatial_roientation",
  "speech_to_text",
  "storage_path",
  "temperature",
  "text_to_speech",
  "threads",
  "unique_id",
  "vibrator",
  "voip",
  "wifi"
]
upper_names = [n.upper() for n in names]

license = """/* https://github.com/takeiteasy/paul

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
along with this program.  If not, see <https://www.gnu.org/licenses/>. */\n\n"""

def generate_header(name):
    p = os.path.join(os.getcwd(), f"native/{name}.h")
    if os.path.exists(p):
        return
    uname = name.upper()
    with open(p, "w") as fh:
        fh.write(license)
        fh.write(f"#ifndef PAUL_{uname}_HEAD\n")
        fh.write(f"#define PAUL_{uname}_HEAD\n")
        fh.write("#ifdef __cplusplus\n")
        fh.write("extern \"C\" {\n")
        fh.write("#endif\n\n")
        fh.write("#ifdef __cplusplus\n")
        fh.write("}\n")
        fh.write(f"#endif // PAUL_{uname}_HEAD\n")

platforms = [
  "android",
  "ios",
  "linux",
  "macos",
  "web",
  "windows"
]

def generate_source(name):
    for p in platforms:
        pa = os.path.join(os.getcwd(), f"native/{p}/{name}.c")
        if not os.path.exists(pa):
            with open(pa, "w") as fh:
                fh.write(license)
                fh.write(f"#include \"../{name}.h\"\n")

output = []

for n in upper_names:
    on = [nn for nn in upper_names if n != nn]
    block = [
      f"#ifdef PAUL_ONLY_{n}",
      "\n".join([f"#define PAUL_NO_{nn}" for nn in on]),
      f"#endif // PAUL_ONLY_{n}\n"
    ]
    output.append(block)

for n in upper_names:
    ln = n.lower()
    header = f"\"native/{ln}.h\""
    block = [
      f"#if !defined(PAUL_NO_{n}) && __has_include({header})",
      f"#include {header}",
      "#endif"
    ]
    generate_header(ln)
    generate_source(ln)
    output.append(block)

with open(os.path.join(os.getcwd(), "paul.h"), "r+") as fh:
    lines = [l.rstrip() for l in fh.readlines()]
    header = []
    footer = []
    state = 0
    for line in lines:
        match state:
            case 0:
                if line == "// BEGIN INCLUDES":
                    state = 1
                header.append(line)
            case 1:
                if line == "// END INCLUDES":
                    state = 2
                    footer.append(line)
            case _:
                footer.append(line)

    fh.seek(0)
    fh.truncate()

    fh.write("\n".join(header) + "\n")
    for block in output:
        fh.write("\n".join(block) + "\n")
    fh.write("\n".join(footer))
