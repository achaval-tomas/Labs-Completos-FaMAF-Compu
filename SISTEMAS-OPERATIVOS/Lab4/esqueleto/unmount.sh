#!/bin/bash

# Check if the script is run as root
if [[ $EUID -ne 0 ]]; then
  echo "This script must be run as root (use sudo)."
  exit 1
fi

# Define the mountpoint to unmount
MOUNTPOINT="$1"

# Try unmounting using fusermount
if fusermount -uq "$MOUNTPOINT"; then
  echo "Unmounted using fusermount."
  exit 0
fi

# If fusermount fails, try unmounting using umount
if umount -q "$MOUNTPOINT"; then
  echo "Unmounted using umount."
  exit 0
fi

# If both fail, check if the mountpoint is not mounted
if ! mountpoint -q "$MOUNTPOINT"; then
  echo "The mountpoint is not mounted."
  exit 0
else
  echo "Failed to unmount."
  echo "The mountpoint is still mounted."
  exit 1
fi
