 #pragma once
#include <stdint.h>

// Theme color definitions for the display
// Use these constants throughout your app for consistent styling

// Basic colors
#define COLOR_BLACK      0x000000
#define COLOR_WHITE      0xFFFFFF
#define COLOR_RED        0xFF0000
#define COLOR_GREEN      0x00FF00
#define COLOR_BLUE       0x0000FF
#define COLOR_YELLOW     0xFFFF00
#define COLOR_CYAN       0x00FFFF
#define COLOR_MAGENTA    0xFF00FF
#define COLOR_GRAY       0x808080
#define COLOR_DARKGRAY   0x404040
#define COLOR_LIGHTGRAY  0xC0C0C0

// Theme-specific colors
#define COLOR_PRIMARY    0x1E90FF  // Dodger Blue
#define COLOR_SECONDARY  0x32CD32  // Lime Green
#define COLOR_ACCENT     0xFFD700  // Gold
#define COLOR_BACKGROUND 0x222222  // Dark background
#define COLOR_SURFACE    0x333333  // Card/Panel background
#define COLOR_ERROR      0xB22222  // Firebrick
#define COLOR_WARNING    0xFFA500  // Orange
#define COLOR_SUCCESS    0x228B22  // Forest Green


// Theme struct for easy switching between normal and dark themes
struct Theme {
	uint32_t background;
	uint32_t surface;
	uint32_t text;
	uint32_t accent;
	uint32_t secondary;
	uint32_t error;
	uint32_t warning;
	uint32_t success;
};

// Normal theme
const Theme THEME_NORMAL = {
	COLOR_WHITE,      // background
	COLOR_LIGHTGRAY,  // surface
	COLOR_BLACK,      // text
	COLOR_ACCENT,     // accent
	COLOR_SECONDARY,  // secondary
	COLOR_ERROR,      // error
	COLOR_WARNING,    // warning
	COLOR_SUCCESS     // success
};

// Dark theme
const Theme THEME_DARK = {
	COLOR_BACKGROUND, // background
	COLOR_SURFACE,    // surface
	COLOR_WHITE,      // text
	COLOR_ACCENT,     // accent
	COLOR_SECONDARY,  // secondary
	COLOR_ERROR,      // error
	COLOR_WARNING,    // warning
	COLOR_SUCCESS     // success
};
