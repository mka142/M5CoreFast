# M5Stack CoreS3 Multi-Device Deployment

Simple deployment tool for uploading firmware to multiple M5Stack CoreS3 devices connected via USB hub.

**Key Feature:** Each device gets a unique `DEVICE_ID` defined at compile time, so you don't need to rebuild 32 times!

## How It Works

The script uses PlatformIO's `--build-flag` option to set a unique `DEVICE_ID` preprocessor macro for each device during upload. This means:
- ✓ Build once
- ✓ Upload to multiple devices with different IDs
- ✓ No full rebuilds needed (only relinks with new ID)
- ✓ Each device knows its unique identifier

## Requirements

- Python 3.6+
- PlatformIO CLI installed (`pip install platformio`)
- M5Stack devices connected via USB

## Usage

### List Connected Devices
```bash
./deploy.py list
```
Shows all detected M5Stack devices (usually `/dev/ttyACM*` or `/dev/ttyUSB*`)

### Build Firmware
```bash
./deploy.py build
```
Builds the firmware for `m5stack-cores3` environment using PlatformIO.

### Upload to All Devices
```bash
./deploy.py upload
```
Automatically detects all connected devices and uploads firmware to each one sequentially.
Each device gets a unique ID: `device_001`, `device_002`, `device_003`, etc.

### Upload to Specific Device
```bash
# Auto-generated ID
./deploy.py upload /dev/ttyACM0

# Custom ID
./deploy.py upload /dev/ttyACM0 my_custom_device_id
```
Upload firmware to a single specific device with optional custom ID.

## Device IDs

When you upload to all devices, they automatically get IDs:
- `/dev/ttyACM0` → `device_001`
- `/dev/ttyACM1` → `device_002`
- `/dev/ttyACM2` → `device_003`
- etc.

The device ID is:
- Set at compile time via `-D DEVICE_ID="device_xxx"`
- Available in your code as `DEVICE_ID` macro
- Printed to Serial at startup
- Used as MQTT Client ID automatically

## Workflow Example

```bash
# 1. Check connected devices
./deploy.py list

# 2. Build firmware
./deploy.py build

# 3. Upload to all devices
./deploy.py upload
```

## One-Liner
```bash
./deploy.py build && ./deploy.py upload
```

## Troubleshooting

**No devices found:**
- Check USB connections
- Ensure devices are powered on
- Check permissions: `sudo usermod -a -G dialout $USER` (then logout/login)

**Build failed:**
- Run `platformio run -e m5stack-cores3` manually to see detailed errors
- Check that you're in the project root directory

**Upload failed:**
- Try unplugging and reconnecting the device
- Check if device is in use by another program (Serial Monitor, etc.)
- Try uploading to one device at a time

## Notes

- Devices are uploaded sequentially with 1 second delay between each
- The script automatically changes to the project root directory
- Color-coded output: ✓ (green) = success, ⚠ (yellow) = warning, ✗ (red) = error
