#!/usr/bin/env python3
"""
Convert PNG sponsor logos to LVGL C image format.
Requires: pip install pillow

Usage: python3 convert_logos.py
"""

from PIL import Image, ImageEnhance
import os
import sys

def png_to_lvgl_c(png_path, output_name, max_width=280, max_height=180):
    """Convert PNG to LVGL C array format (RGB565) with dark background"""
    
    print(f"Converting: {png_path}")
    
    # Open image
    img = Image.open(png_path)
    
    print(f"  Source: {img.width}x{img.height} ({img.mode})")
    
    # If source is small, upscale first for better quality (anti-aliasing)
    MIN_SOURCE_SIZE = 800
    if img.width < MIN_SOURCE_SIZE or img.height < MIN_SOURCE_SIZE:
        scale = MIN_SOURCE_SIZE / max(img.width, img.height)
        new_w = int(img.width * scale)
        new_h = int(img.height * scale)
        print(f"  Upscaling low-res source: {img.width}x{img.height} → {new_w}x{new_h}")
        img = img.resize((new_w, new_h), Image.Resampling.LANCZOS)
    
    # Calculate aspect-preserving resize to fill space better
    img_ratio = img.width / img.height
    target_ratio = max_width / max_height
    
    if img_ratio > target_ratio:
        # Image is wider - fit to width
        new_width = max_width
        new_height = int(max_width / img_ratio)
    else:
        # Image is taller - fit to height
        new_height = max_height
        new_width = int(max_height * img_ratio)
    
    # Resize with high quality (LANCZOS for downscaling from upscaled source)
    img = img.resize((new_width, new_height), Image.Resampling.LANCZOS)
    
    # Keep RGBA for alpha blending
    if img.mode != 'RGBA':
        img = img.convert('RGBA')
    
    # FIRST: EXTREME contrast for dark theme visibility
    enhancer = ImageEnhance.Contrast(img)
    img = enhancer.enhance(3.0)  # było 2.2, teraz 3.0 - ekstremalny kontrast
    
    # THEN: Keep brightness neutral (avoid washout OR too dark)
    enhancer = ImageEnhance.Brightness(img)
    img = enhancer.enhance(1.0)  # było 0.95, teraz 1.0 - neutralnie
    
    # Add heavy sharpness for crisp text
    enhancer = ImageEnhance.Sharpness(img)
    img = enhancer.enhance(2.5)  # było 2.0, teraz 2.5 - bardzo ostre
    
    width, height = img.size
    pixels = img.load()
    
    print(f"  Size: {width}x{height} (RGB565A8 with alpha blending)")
    
    # Generate C array with RGB565A8 (all RGB, then all alpha - LVGL v9 format)
    c_code = f"""// Auto-generated from {os.path.basename(png_path)}
// Size: {width}x{height} - RGB565A8 with alpha blending

#include <lvgl.h>

static const uint8_t {output_name}_map[] = {{
"""
    
    # FIRST: All RGB565 pixels (as bytes, little-endian)
    pixel_count = 0
    for y in range(height):
        c_code += "    "
        for x in range(width):
            r, g, b, a = pixels[x, y]
            
            # Convert to RGB565
            r5 = (r >> 3) & 0x1F
            g6 = (g >> 2) & 0x3F
            b5 = (b >> 3) & 0x1F
            rgb565 = (r5 << 11) | (g6 << 5) | b5
            
            # Little-endian: low byte, then high byte
            c_code += f"0x{rgb565 & 0xFF:02x}, 0x{(rgb565 >> 8) & 0xFF:02x}, "
            pixel_count += 1
            
            if pixel_count % 6 == 0:
                c_code += "\n    "
    
    c_code += "\n    // Alpha channel\n"
    
    # SECOND: All alpha values (1 byte per pixel)
    pixel_count = 0
    for y in range(height):
        c_code += "    "
        for x in range(width):
            r, g, b, a = pixels[x, y]
            c_code += f"0x{a:02x}, "
            pixel_count += 1
            
            if pixel_count % 16 == 0:
                c_code += "\n    "
    
    c_code += f"""
}};

const lv_image_dsc_t {output_name} = {{
    .header = {{
        .magic = LV_IMAGE_HEADER_MAGIC,
        .cf = LV_COLOR_FORMAT_RGB565A8,
        .flags = 0,
        .w = {width},
        .h = {height},
        .stride = {width * 2},
        .reserved_2 = 0,
    }},
    .data = {output_name}_map,
    .data_size = sizeof({output_name}_map),
}};
"""
    
    return c_code

def main():
    img_dir = "../img/sponsor"  # HIGH-RES logos
    output_file = "../include/sponsor_logos.h"
    
    if not os.path.exists(img_dir):
        print(f"ERROR: Directory {img_dir} not found!")
        sys.exit(1)
    
    print("=" * 60)
    print("LVGL Sponsor Logo Converter - HIGH QUALITY")
    print("=" * 60)
    
    # Logo mapping (HIGH-RES versions) - 6 logos only
    logos = {
        "wroclaw-b.png": ("logo_wroclaw", 300, 200),  # Wrocław - DUŻE
        "kannm.png": ("logo_kannm", 300, 200),        # Kampania - DUŻE  
        "wca.png": ("logo_wca", 300, 200),            # WCA - DUŻE (jedyne ładne)
        "am1.png": ("logo_amkl", 280, 180),           # Akademia - standard
        "knakitm.png": ("logo_knakitm", 300, 200),    # Koło - DUŻE (było niewidoczne)
        "sknm.png": ("logo_sknm", 280, 180),          # SKNM - standard
    }
    
    header = """#pragma once
// Auto-generated sponsor logos for LVGL
// DO NOT EDIT MANUALLY - regenerate with convert_logos.py

#include <lvgl.h>

"""
    
    # Declare externals
    for logo_data in logos.values():
        var_name = logo_data[0] if isinstance(logo_data, tuple) else logo_data
        header += f"extern const lv_image_dsc_t {var_name};\n"
    
    # Generate .c file with implementations
    c_file = """// Auto-generated sponsor logos for LVGL
#include "images/sponsor_logos.h"

"""
    
    for png_file, logo_data in logos.items():
        png_path = os.path.join(img_dir, png_file)
        if os.path.exists(png_path):
            if isinstance(logo_data, tuple):
                var_name, width, height = logo_data
                c_code = png_to_lvgl_c(png_path, var_name, width, height)
            else:
                c_code = png_to_lvgl_c(png_path, logo_data)
            c_file += c_code + "\n\n"
        else:
            print(f"  WARNING: {png_path} not found, skipping")
    
    # Write header file
    with open(output_file, 'w') as f:
        f.write(header)
    
    print(f"\n✓ Header written to: {output_file}")
    
    # Write .c file
    c_output = output_file.replace('.h', '.c')
    with open(c_output, 'w') as f:
        f.write(c_file)
    
    print(f"✓ Implementation written to: {c_output}")
    print("\n" + "=" * 60)
    print("Done! Add these to platformio.ini build_src_filter:")
    print("  +<../include/*.c>")
    print("=" * 60)

if __name__ == "__main__":
    main()
