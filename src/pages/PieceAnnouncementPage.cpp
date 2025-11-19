#include "PieceAnnouncementPage.h"
#include <polish_fonts.h>

// Static member definitions
lv_obj_t* PieceAnnouncementPage::composer_label = nullptr;
lv_obj_t* PieceAnnouncementPage::piece_label = nullptr;
lv_obj_t* PieceAnnouncementPage::performers_label = nullptr;
lv_obj_t* PieceAnnouncementPage::description_label = nullptr;
EventSchema PieceAnnouncementPage::eventPayload = {};  // Initialize payload storage

lv_obj_t* PieceAnnouncementPage::create() {
    // Create main screen
    lv_obj_t* screen = lv_obj_create(nullptr);
    lv_obj_set_size(screen, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);  // Black background
    lv_obj_set_style_border_width(screen, 0, 0);
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
    
    // Create scrollable container for content
    lv_obj_t* scroll_container = lv_obj_create(screen);
    lv_obj_set_size(scroll_container, 320, 240);
    lv_obj_set_pos(scroll_container, 0, 0);
    lv_obj_set_style_bg_color(scroll_container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(scroll_container, 0, 0);
    lv_obj_set_style_pad_all(scroll_container, 20, 0);
    lv_obj_set_flex_flow(scroll_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scroll_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(scroll_container, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(scroll_container, LV_DIR_VER);
    
    // Composer label (white, bold, 32pt)
    composer_label = lv_label_create(scroll_container);
    lv_obj_set_style_text_font(composer_label, &montserrat_32_polish, 0);
    lv_obj_set_style_text_color(composer_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(composer_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(composer_label, 280);
    lv_label_set_long_mode(composer_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(composer_label, "Kompozytor nieznany");
    
    // Piece label (white, 20pt)
    piece_label = lv_label_create(scroll_container);
    lv_obj_set_style_text_font(piece_label, &montserrat_20_polish, 0);
    lv_obj_set_style_text_color(piece_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(piece_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(piece_label, 280);
    lv_label_set_long_mode(piece_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(piece_label, "Utwór nieznany");
    lv_obj_set_style_pad_top(piece_label, 15, 0);
    
    // Performers label (white, 14pt)
    performers_label = lv_label_create(scroll_container);
    lv_obj_set_style_text_font(performers_label, &montserrat_14_polish, 0);
    lv_obj_set_style_text_color(performers_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(performers_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(performers_label, 280);
    lv_label_set_long_mode(performers_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(performers_label, "");
    lv_obj_set_style_pad_top(performers_label, 15, 0);
    
    // Description label (gray, 14pt)
    description_label = lv_label_create(scroll_container);
    lv_obj_set_style_text_font(description_label, &montserrat_14_polish, 0);
    lv_obj_set_style_text_color(description_label, lv_color_hex(0x999999), 0);  // Gray
    lv_obj_set_style_text_align(description_label, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_width(description_label, 280);
    lv_label_set_long_mode(description_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(description_label, "");
    lv_obj_set_style_pad_top(description_label, 20, 0);
    lv_obj_set_style_pad_bottom(description_label, 20, 0);
    
    return screen;
}

void PieceAnnouncementPage::setComposer(const char* composer) {
    if (composer_label) {
        lv_label_set_text(composer_label, composer);
    }
}

void PieceAnnouncementPage::setPiece(const char* piece) {
    if (piece_label) {
        lv_label_set_text(piece_label, piece);
    }
}

void PieceAnnouncementPage::setPerformers(const char* performers) {
    if (performers_label) {
        lv_label_set_text(performers_label, performers);
    }
}

void PieceAnnouncementPage::setDescription(const char* description) {
    if (description_label) {
        lv_label_set_text(description_label, description);
    }
}

void PieceAnnouncementPage::setPayload(const EventSchema& payload) {
    eventPayload = payload;
    
    // Extract data from payload
    const JsonObject& data = payload.payload;
    
    if (!data.isNull()) {
        // Update composer
        if (data.containsKey("composerName")) {
            setComposer(data["composerName"].as<const char*>());
        } else {
            setComposer("Kompozytor nieznany");
        }
        
        // Update piece title
        if (data.containsKey("pieceTitle")) {
            setPiece(data["pieceTitle"].as<const char*>());
        } else {
            setPiece("Utwór nieznany");
        }
        
        // Update performers
        if (data.containsKey("performers")) {
            setPerformers(data["performers"].as<const char*>());
        } else {
            setPerformers("");
        }
        
        // Update description
        if (data.containsKey("pieceDescription")) {
            setDescription(data["pieceDescription"].as<const char*>());
        } else {
            setDescription("");
        }
    } else {
        // No payload, use defaults
        setComposer("Kompozytor nieznany");
        setPiece("Utwór nieznany");
        setPerformers("");
        setDescription("");
    }
    
    Serial.println("=== PieceAnnouncementPage setPayload ===");
    Serial.print("Concert ID: ");
    Serial.println(payload.concertId.c_str());
    Serial.print("Event Type: ");
    Serial.println(payload.eventType.c_str());
    if (!data.isNull()) {
        Serial.println("Payload contains:");
        serializeJsonPretty(data, Serial);
        Serial.println();
    }
}

const EventSchema& PieceAnnouncementPage::getPayload() {
    return eventPayload;
}
