#!/bin/bash
# Quick Start Guide - Run this after cloning

echo "╔════════════════════════════════════════════════╗"
echo "║  Sponsor Carousel - Quick Start                ║"
echo "╚════════════════════════════════════════════════╝"
echo ""

# Step 1: Convert logos
echo "📸 Step 1: Converting sponsor logos..."
cd tools
chmod +x convert.sh
./convert.sh

if [ $? -ne 0 ]; then
    echo "❌ Conversion failed!"
    exit 1
fi

cd ..

# Step 2: Build
echo ""
echo "🔨 Step 2: Building firmware..."
pio run

if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi

echo ""
echo "✅ SUCCESS! Ready to use SponsorCarousel"
echo ""
echo "📝 Next steps:"
echo "   1. Add to main.cpp:"
echo '      #include "pages/SponsorsPage.h"'
echo ""
echo "   2. In setup():"
echo '      lv_obj_t *sponsors = SponsorsPage::create();'
echo '      navigator.registerScreen(SPONSORS, sponsors);'
echo '      navigator.showPage(SPONSORS);'
echo ""
echo "   3. Upload:"
echo '      pio run --target upload'
echo ""
