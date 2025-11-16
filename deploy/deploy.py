#!/usr/bin/env python3
"""
M5Stack CoreS3 Multi-Device Deployment Tool

Simple tool to detect, build, and upload firmware to multiple M5Stack devices.
Each device gets a unique DEVICE_ID defined at compile time.

Usage:
    ./deploy.py list                    - List all connected M5Stack devices
    ./deploy.py build                   - Build firmware for m5stack-cores3
    ./deploy.py upload                  - Upload to all devices (auto ID: device_001, device_002, ...)
    ./deploy.py upload /dev/ttyACM0     - Upload to specific device (auto ID)
    ./deploy.py upload /dev/ttyACM0 mydevice  - Upload with custom device ID
"""

import subprocess
import sys
import os
import time
from pathlib import Path

# ANSI color codes
GREEN = '\033[92m'
YELLOW = '\033[93m'
RED = '\033[91m'
BLUE = '\033[94m'
RESET = '\033[0m'

def print_success(msg):
    print(f"{GREEN}✓{RESET} {msg}")

def print_info(msg):
    print(f"{BLUE}ℹ{RESET} {msg}")

def print_warning(msg):
    print(f"{YELLOW}⚠{RESET} {msg}")

def print_error(msg):
    print(f"{RED}✗{RESET} {msg}")

def get_project_root():
    """Get the project root directory (one level up from deploy/)"""
    return Path(__file__).parent.parent

def find_platformio():
    """Find platformio executable in common locations"""
    # Try common locations
    locations = [
        'platformio',  # In PATH
        'pio',  # Short alias
        str(Path.home() / '.platformio' / 'penv' / 'bin' / 'platformio'),
        str(Path.home() / '.platformio' / 'penv' / 'bin' / 'pio'),
    ]
    
    for cmd in locations:
        try:
            result = subprocess.run(
                [cmd, '--version'],
                capture_output=True,
                text=True,
                timeout=5
            )
            if result.returncode == 0:
                return cmd
        except (FileNotFoundError, subprocess.TimeoutExpired):
            continue
    
    return None

def detect_m5_devices():
    """Detect all connected M5Stack devices (usually ttyACM* or ttyUSB*)"""
    devices = []
    dev_dir = Path('/dev')
    
    # Look for typical M5Stack device patterns
    patterns = ['ttyACM*', 'ttyUSB*']
    
    for pattern in patterns:
        for device in dev_dir.glob(pattern):
            # Basic check if device is accessible
            if device.exists():
                devices.append(str(device))
    
    return sorted(devices)

def list_devices():
    """List all detected M5Stack devices"""
    print_info("Scanning for M5Stack devices...")
    devices = detect_m5_devices()
    
    if not devices:
        print_warning("No devices found!")
        print_info("Make sure your M5Stack devices are connected via USB")
        return []
    
    print_success(f"Found {len(devices)} device(s):")
    for i, device in enumerate(devices, 1):
        print(f"  {i}. {device}")
    
    return devices

def build_firmware():
    """Build the firmware using PlatformIO"""
    print_info("Building firmware for m5stack-cores3...")
    
    pio_cmd = find_platformio()
    if not pio_cmd:
        print_error("PlatformIO not found!")
        print_info("Install it with: pip install platformio")
        print_info("Or check ~/.platformio/penv/bin/platformio")
        return False
    
    project_root = get_project_root()
    os.chdir(project_root)
    
    try:
        result = subprocess.run(
            [pio_cmd, 'run', '--environment', 'm5stack-cores3'],
            capture_output=True,
            text=True
        )
        
        if result.returncode == 0:
            print_success("Build successful!")
            return True
        else:
            print_error("Build failed!")
            print(result.stderr)
            return False
    except Exception as e:
        print_error(f"Build error: {e}")
        return False

def upload_to_device(device, device_id=None):
    """Upload firmware to a specific device"""
    device_name = device_id if device_id else device.split('/')[-1]
    print_info(f"Uploading to {device} (ID: {device_name})...")
    
    pio_cmd = find_platformio()
    if not pio_cmd:
        print_error("PlatformIO not found!")
        return False
    
    project_root = get_project_root()
    os.chdir(project_root)
    
    # Build command with optional device ID
    cmd = [pio_cmd, 'run', '--target', 'upload',
           '--environment', 'm5stack-cores3',
           '--upload-port', device]
    
    # Add device ID as environment variable if provided
    env = os.environ.copy()
    if device_id:
        env['PLATFORMIO_BUILD_FLAGS'] = f'-D DEVICE_ID=\\"{device_id}\\"'
    
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            env=env
        )
        
        if result.returncode == 0:
            print_success(f"Upload to {device} (ID: {device_name}) "
                         "successful!")
            return True
        else:
            print_error(f"Upload to {device} failed!")
            # Show last few lines of error
            error_lines = result.stderr.split('\n')
            for line in error_lines[-10:]:
                if line.strip():
                    print(f"  {line}")
            return False
    except Exception as e:
        print_error(f"Upload error: {e}")
        return False

def upload_all():
    """Upload firmware to all detected devices"""
    devices = detect_m5_devices()
    
    if not devices:
        print_warning("No devices found to upload to!")
        return False
    
    print_info(f"Uploading to {len(devices)} device(s)...")
    
    success_count = 0
    fail_count = 0
    
    # Generate device IDs (device_001, device_002, etc.)
    for idx, device in enumerate(devices, 1):
        device_id = f"device_{idx:03d}"
        if upload_to_device(device, device_id):
            success_count += 1
        else:
            fail_count += 1
        
        # Small delay between uploads
        if device != devices[-1]:
            time.sleep(1)
    
    print("\n" + "="*50)
    print_info(f"Upload complete: {success_count} success, "
               f"{fail_count} failed")
    
    return fail_count == 0


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    
    command = sys.argv[1].lower()
    
    if command == 'list':
        list_devices()
    
    elif command == 'build':
        success = build_firmware()
        sys.exit(0 if success else 1)
    
    elif command == 'upload':
        # Check if specific device is provided
        if len(sys.argv) > 2:
            device = sys.argv[2]
            if not os.path.exists(device):
                print_error(f"Device {device} not found!")
                sys.exit(1)
            # Optional: custom device ID as third argument
            device_id = sys.argv[3] if len(sys.argv) > 3 else None
            success = upload_to_device(device, device_id)
        else:
            success = upload_all()
        
        sys.exit(0 if success else 1)
    
    else:
        print_error(f"Unknown command: {command}")
        print(__doc__)
        sys.exit(1)


if __name__ == '__main__':
    main()
