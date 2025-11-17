#!/bin/bash
# Quick converter for sponsor logos

cd "$(dirname "$0")"

echo "=================================="
echo "Sponsor Logo Converter for LVGL"
echo "=================================="
echo ""

# Check if Python is available
if ! command -v python3 &> /dev/null; then
    echo "ERROR: python3 not found!"
    exit 1
fi

# Check if Pillow is installed
if ! python3 -c "import PIL" 2>/dev/null; then
    echo "Installing Pillow..."
    echo ""
    echo "Option 1 (recommended): brew install pillow"
    echo "Option 2: python3 -m pip install --break-system-packages pillow"
    echo ""
    
    # Try brew first
    if command -v brew &> /dev/null; then
        echo "Using Homebrew..."
        brew install pillow 2>/dev/null || python3 -m pip install --break-system-packages pillow
    else
        echo "Homebrew not found, using pip..."
        python3 -m pip install --break-system-packages pillow
    fi
fi

# Run converter
echo ""
echo "Converting PNG logos to LVGL format..."
python3 convert_logos.py

if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Success! Files generated:"
    echo "  - include/sponsor_logos.h"
    echo "  - include/sponsor_logos.c"
    echo ""
    echo "Next steps:"
    echo "  1. Verify platformio.ini has: +<../include/*.c>"
    echo "  2. Build project: pio run"
    echo ""
else
    echo ""
    echo "✗ Conversion failed. Check errors above."
    exit 1
fi
