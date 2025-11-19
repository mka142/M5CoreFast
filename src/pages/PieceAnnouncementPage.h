#pragma once
#include <Arduino.h>
#include <lvgl.h>
#include <MQTTCommon.h>  // For EventSchema

class PieceAnnouncementPage {
public:
    static lv_obj_t* create();
    static void setComposer(const char* composer);
    static void setPiece(const char* piece);
    static void setPerformers(const char* performers);
    static void setDescription(const char* description);
    
    // Payload methods for MQTT integration
    static void setPayload(const EventSchema& payload);
    static const EventSchema& getPayload();
    
private:
    static lv_obj_t* composer_label;
    static lv_obj_t* piece_label;
    static lv_obj_t* performers_label;
    static lv_obj_t* description_label;
    static EventSchema eventPayload;  // Holds event data from MQTT
};
