# Efficient Multi-Device Deployment with Unique IDs

## The Problem Solved

You don't want to rebuild 32 times just to change the device ID. This solution lets you:
- **Build once** 
- **Upload to multiple devices** with different IDs
- **No full rebuilds** (PlatformIO only relinks with new flags)

## How It Works

### 1. PlatformIO Build Flags
```ini
# platformio.ini
build_flags = 
  -D DEVICE_ID="${PLATFORMIO_BUILD_FLAGS}"
```

### 2. Dynamic ID at Upload Time
```bash
platformio run --target upload --upload-port /dev/ttyACM0 \
  --build-flag '-D DEVICE_ID="device_001"'
```

This triggers a **fast relink** (~5-10 seconds) instead of full rebuild (~2-3 minutes).

### 3. Usage in Code
```cpp
// main.cpp
#ifndef DEVICE_ID
#define DEVICE_ID "device_default"
#endif

void setup() {
    Serial.print("Device ID: ");
    Serial.println(DEVICE_ID);
    
    // Use as MQTT client ID
    MQTTAdapter mqttAdapter(MQTT_SERVER, MQTT_PORT, DEVICE_ID);
}
```

## Deployment Script Features

### Automatic Device ID Assignment
```bash
./deploy.py upload
```
- Detects all devices
- Assigns IDs: device_001, device_002, device_003, ...
- Uploads to each sequentially

### Custom Device IDs
```bash
./deploy.py upload /dev/ttyACM0 production_display_01
./deploy.py upload /dev/ttyACM1 testing_device_alpha
```

### Typical Workflow
```bash
# 1. List devices
./deploy.py list
# Found 8 devices

# 2. Build once
./deploy.py build
# ✓ Build successful! (2-3 minutes)

# 3. Upload to all with unique IDs
./deploy.py upload
# ✓ Upload to /dev/ttyACM0 (ID: device_001) successful! (10 seconds)
# ✓ Upload to /dev/ttyACM1 (ID: device_002) successful! (10 seconds)
# ✓ Upload to /dev/ttyACM2 (ID: device_003) successful! (10 seconds)
# ... (8 total)
```

## Performance

| Operation | Time per Device | Time for 32 Devices |
|-----------|----------------|---------------------|
| **Full rebuild** | ~3 min | ~96 minutes |
| **Relink with new ID** | ~10 sec | ~5-6 minutes |

**Result: 16x faster deployment for multiple devices!**

## Technical Details

### Why It's Fast

PlatformIO only needs to:
1. Update the preprocessor definition
2. Recompile files that use DEVICE_ID (just main.cpp)
3. Relink the binary

It does NOT:
- Recompile all libraries
- Rebuild framework code
- Reprocess dependencies

### Build Flag Propagation

```bash
--build-flag '-D DEVICE_ID="my_device"'
```
↓
```ini
build_flags = -D DEVICE_ID="my_device"
```
↓
```cpp
#define DEVICE_ID "my_device"
```

## Examples

### Upload with Sequential IDs
```bash
./deploy.py upload
# device_001, device_002, device_003, ...
```

### Upload with Room Names
```bash
for port in /dev/ttyACM{0..7}; do
    room="room_$(basename $port | tr -d 'ttyACM')"
    ./deploy.py upload $port $room
done
# room_0, room_1, room_2, ...
```

### Upload with Location Codes
```bash
./deploy.py upload /dev/ttyACM0 warsaw_hall_a_001
./deploy.py upload /dev/ttyACM1 warsaw_hall_a_002
./deploy.py upload /dev/ttyACM2 warsaw_hall_b_001
```

## Verification

Each device prints its ID at startup:
```
Starting system...
Device ID: device_003
MQTT connected with client ID: device_003
```

Check the Serial Monitor after upload to verify the correct ID was programmed.
