#!/bin/bash
# Simple wrapper for deploy.py

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

python3 "$SCRIPT_DIR/deploy.py" "$@"
