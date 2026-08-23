import os

BASE = "/run/media/ymk/new 2T/other/backups/arch_home_2026_aug_23(before omrachy)/other/progects/cydos"
APPS = os.path.join(BASE, "sdcard/apps")
OUT = os.path.join(BASE, "firmware/cydos/sdpack.h")

entries = []
for name in sorted(os.listdir(APPS)):
    d = os.path.join(APPS, name)
    if not os.path.isdir(d):
        continue
    for f in ["app.lua", "manifest.txt", "icon.jpg"]:
        p = os.path.join(d, f)
        if os.path.exists(p):
            entries.append((f"/apps/{name}/{f}", open(p, "rb").read()))

with open(OUT, "w") as h:
    h.write("#pragma once\n#include <pgmspace.h>\n\n")
    h.write("struct EmbedFile { const char *path; const uint8_t *data; uint32_t len; };\n\n")
    for i, (path, data) in enumerate(entries):
        var = f"sdfile{i}"
        h.write(f"static const uint8_t {var}[] PROGMEM = {{\n")
        for j in range(0, len(data), 20):
            chunk = ", ".join(str(b) for b in data[j:j+20])
            h.write(f"  {chunk},\n")
        h.write("};\n\n")
    h.write("static const EmbedFile EMBED_FILES[] = {\n")
    for i, (path, data) in enumerate(entries):
        h.write(f'  {{ "{path}", sdfile{i}, {len(data)} }},\n')
    h.write("};\n")
    h.write(f"#define EMBED_COUNT {len(entries)}\n")

print(f"packed {len(entries)} files ({sum(len(d) for _, d in entries)} bytes)")
