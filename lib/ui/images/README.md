# Image Resources

This directory contains all converted image resources for LVGL in C array format.

## Files

### Fader Control
- `fader_knob.c/h` - Vertical fader knob graphic (199×52 px, ARGB8888)
  - Used in: `SliderDemoPage`
  - Source: `img/fader-199x52.png`
  - Size: ~41 KB

### Musical Elements
- `note.c/h` - Musical note icon (ARGB8888)
  - Used in: `BeforeConcertPage` (animated rotation)
  - Source: `img/note.png`
  - Size: ~221 KB

### Sponsor Logos
- `sponsor_logos.c/h` - All sponsor logos (ARGB8888)
  - Used in: `SponsorCarousel`, `SponsorsPage`
  - Sources: `img/sponsor/*.png`
  - Count: 6 logos
  - Size: ~3.4 MB
  - Logos: AM1, KANNM, KNAKITM, NFM, SKNM, WCA, Wrocław

## Format

All images use **ARGB8888** (32-bit) format with BGRA byte order (little endian):
```c
// Byte order in memory: Blue, Green, Red, Alpha
0xBB, 0xGG, 0xRR, 0xAA
```

This format ensures:
- Correct color rendering on ESP32-S3
- Proper alpha blending
- No interleaving artifacts

## Usage

Include images in your code:
```cpp
#include <images/fader_knob.h>
#include <images/sponsor_logos.h>
#include <images/note.h>

// Use with LVGL
lv_image_set_src(img_obj, &fader_knob);
lv_image_set_src(logo_obj, &logo_am1);
```

## Regeneration

To regenerate images from PNG sources:
```bash
# Fader knob
python3 tools/convert_fader.py

# Sponsor logos
python3 tools/convert_logos.py

# Musical note
python3 tools/convert_note.py
```

## Memory Usage

Total size: ~3.9 MB uncompressed
- Stored in Flash (not RAM)
- Loaded to RAM only when displayed
- ESP32-S3 has 16 MB Flash, plenty of space
