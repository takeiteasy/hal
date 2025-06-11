import os

names = [
  "accelerometer",
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
  "spatial_orientation",
  "speech_to_text",
  "temperature",
  "text_to_speech",
  "threads",
  "unique_id",
  "vibrator",
  "voip",
  "wifi"
]
upper_names = [n.upper() for n in names]

ignore_files = [
  "accelerometer",
  "clipboard",
  "threads"
]

files_backends = {
  "linux": ["clipboard"]
}

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
    if name in ignore_files:
        return
    p = os.path.join(os.getcwd(), f"native/{name}.h")
    uname = name.upper()
    with open(p, "w") as fh:
        fh.write(license)
        fh.write(f"#ifndef PAUL_{uname}_HEAD\n")
        fh.write(f"#define PAUL_{uname}_HEAD\n")
        fh.write("#ifdef __cplusplus\n")
        fh.write("extern \"C\" {\n")
        fh.write("#endif\n\n")
        fh.write(f"#define PAUL_ONLY_{name.upper()}\n")
        fh.write("#include \"../paul.h\"\n\n")
        fh.write("#ifdef __cplusplus\n")
        fh.write("}\n")
        fh.write("#endif\n")
        fh.write(f"#endif // PAUL_{uname}_HEAD\n")

platforms = [
  "android",
  "ios",
  "linux",
  "macos",
  "web",
  "windows"
]

platform_backends = {
  "linux": ["gtk", "wayland", "x11"]
}

def generate_source(name):
    if name in ignore_files:
        return
    for p in platforms:
        ext = ".m" if p in ["ios", "macos"] else ".c"
        _use_backend = p in files_backends
        pa = os.path.join(os.getcwd(), f"native/{p}/{name}{ext}")
        with open(pa, "w") as fh:
            fh.write(license)
            fh.write(f"#ifndef PAUL_NO_{name.upper()}\n")
            fh.write(f"#include \"../{name}.h\"\n")
            if _use_backend:
                fh.write("#include \"internal.h\"\n\n")
                fh.write("#ifdef ")
                _backends = platform_backends[p] + ["dummy"]
                for b in _backends:
                    bpa = os.path.join(os.getcwd(), f"native/{p}/backends/{b}/{name}{ext}")
                    with open(bpa, "w") as bfh:
                        bfh.write(license)
                        bfh.write(f"#ifndef PAUL_NO_{name.upper()}\n\n")
                        bfh.write(f"#endif // PAUL_NO_{name.upper()}")
                    fh.write(f"PAUL_HAS_{b}\n")
                    fh.write(f"#include \"backends/{b}/{name}{ext}\"")
                    if b == "dummy":
                        fh.write("#endif\n")
                    else:
                        fh.write(f"#elif ")
            else:
                fh.write("\n")
            fh.write(f"#endif // PAUL_NO_{name.upper()}")

output = []

for n in upper_names:
    on = [nn for nn in upper_names if n != nn]
    block = [
      f"#ifdef PAUL_ONLY_{n}",
      "\n".join([f"#define PAUL_NO_{nn}" for nn in on]),
      f"#endif // PAUL_ONLY_{n}\n",
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
