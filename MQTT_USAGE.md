# MQTT Page Control - Testing Guide

## Message Format

Your MQTT messages should follow this EventSchema format:

```json
{
  "concertId": "concert-123",
  "eventType": "home",
  "label": "Switch to Home Page",
  "payload": {
    "additionalData": "optional"
  },
  "position": 1
}
```

## Example Messages

### Switch to Home Page
```json
{
  "concertId": "concert-2025-10-25",
  "eventType": "home",
  "label": "Display Home Screen",
  "payload": {},
  "position": 1
}
```

### Switch to Info Page
```json
{
  "concertId": "concert-2025-10-25",
  "eventType": "info",
  "label": "Show Information Screen",
  "payload": {
    "infoType": "schedule"
  },
  "position": 2
}
```

### Custom Event Type
```json
{
  "concertId": "concert-2025-10-25",
  "eventType": "custom_page",
  "label": "Show Custom Content",
  "payload": {
    "customData": "value",
    "priority": "high"
  },
  "position": 3
}
```

## Testing with MQTT

### Using mosquitto_pub (command line):
```bash
# Switch to home page
mosquitto_pub -h your-mqtt-broker.com -p 1883 -t "concert/events" -m '{"concertId":"test-123","eventType":"home","label":"Test Home","payload":{},"position":1}'

# Switch to info page
mosquitto_pub -h your-mqtt-broker.com -p 1883 -t "concert/events" -m '{"concertId":"test-123","eventType":"info","label":"Test Info","payload":{},"position":2}'
```

### Using MQTT.fx or similar GUI tools:
- Topic: `concert/events`
- Message: Copy the JSON examples above

## Configuration

Update these values in main.cpp:
```cpp
#define MQTT_SERVER "your-actual-broker.com"     // Your MQTT broker
#define MQTT_PORT 1883                           // Standard MQTT port
#define MQTT_CLIENT_ID "m5cores3_concert_display" // Unique client ID
#define MQTT_TOPIC_EVENTS "concert/events"       // Topic to subscribe to
#define MQTT_TOPIC_STATUS "display/status"       // Topic for status updates
```

## Adding New Pages

1. Define new PageID in `lib/ui/PageID.h`:
```cpp
enum PageID {
    PAGE_HOME,
    PAGE_SETTINGS,
    PAGE_INFO,
    PAGE_SCHEDULE,    // New page
    PAGE_ARTISTS,     // New page
};
```

2. Add mapping in main.cpp setup():
```cpp
mqttPageBridge.addPageMapping("schedule", PAGE_SCHEDULE);
mqttPageBridge.addPageMapping("artists", PAGE_ARTISTS);
```

3. Register page with PageManager:
```cpp
pageManager.registerPage(PAGE_SCHEDULE, &schedulePage);
pageManager.registerPage(PAGE_ARTISTS, &artistsPage);
```

## Architecture Benefits

- **Independent MQTT Adapter**: No dependencies on display components
- **Non-blocking**: MQTT processing doesn't block display rendering
- **Queue-based**: Messages are queued and processed smoothly
- **Extensible**: Easy to add new event types and pages
- **Robust**: Handles disconnections and invalid messages gracefully
- **Debuggable**: Comprehensive logging for troubleshooting