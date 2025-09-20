#include "DisplayAdapter.h"

/**
 * @brief Initialize the display hardware and clear the screen.
 */
void DisplayAdapter::begin()
{
    display.init();
    display.startWrite();
    display.fillScreen(TFT_BLACK);
    Serial.println("Display initialized.");
}

/**
 * @brief Configure display settings such as EPD mode and rotation.
 */
void DisplayAdapter::configure()
{
    if (display.isEPD())
    {
        display.setEpdMode(epd_mode_t::epd_fastest);
    }
    if (display.width() < display.height())
    {
        display.setRotation(display.getRotation() ^ 1);
    }
    Serial.println("Display configured.");
}

/**
 * @brief Fill the entire display with the specified color. (default is black)
 * @param color The color to fill the screen with.
 */
void DisplayAdapter::clear(uint32_t color)
{
    display.fillScreen(color);
}

/**
 * @brief Push the current display buffer to the screen.
 *
 * This method sends the contents of the off-screen buffer to the physical display.
 * All drawing operations (text, shapes, etc.) are performed on the buffer first.
 * Calling update() ensures the user only sees fully rendered frames, preventing flicker and partial updates.
 *
 * Usage pattern:
 *   1. Call clear() to reset the buffer.
 *   2. Draw all UI elements for the current frame.
 *   3. Call update() to show the new frame.
 *
 * This approach is essential for smooth, flicker-free UI and works well with waitDisplay() for synchronization.
 */
void DisplayAdapter::update()
{
    display.display();
}

/**
 * @brief Wait for the display to finish updating (synchronization).
 *
 * This method blocks execution until the display hardware has finished processing the previous update.
 * It is useful for synchronizing frame rendering, ensuring that the next frame is not drawn before the previous one is complete.
 *
 * Use this in your main loop before rendering a new frame to avoid tearing, flicker, or overlapping graphics.
 *
 * Usage pattern:
 *   1. Call waitDisplay() to ensure the display is ready.
 *   2. Render the next frame (clear, draw, update).
 *
 * This is especially important for animations or rapid UI changes, where timing and smoothness matter.
 */
void DisplayAdapter::waitDisplay()
{
    display.waitDisplay();
}

/**
 * @brief Get the width of the display in pixels.
 * @return Display width.
 */
int DisplayAdapter::width() { return display.width(); }

/**
 * @brief Get the height of the display in pixels.
 * @return Display height.
 */
int DisplayAdapter::height() { return display.height(); }

/**
 * @brief Get a reference to the underlying M5GFX display object.
 * @return Reference to M5GFX display.
 */
M5GFX &DisplayAdapter::getDisplay() { return display; }