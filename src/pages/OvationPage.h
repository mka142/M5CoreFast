#pragma once
#include <Arduino.h>
#include <lvgl.h>
#include <MQTTAdapter.h>  // For EventSchema

class OvationPage {
public:
    static lv_obj_t* create();
    static void update_animation(lv_timer_t *timer);
    static void setMessage(const char* message);
    
    // Payload methods for MQTT integration
    static void setPayload(const EventSchema& payload);
    static const EventSchema& getPayload();
    
private:
    static lv_obj_t* message_label;
    static lv_obj_t* dot1;
    static lv_obj_t* dot2;
    static lv_obj_t* dot3;
    static int animation_step;
    static lv_timer_t* animation_timer;
    static EventSchema eventPayload;  // Holds event data from MQTT
};
