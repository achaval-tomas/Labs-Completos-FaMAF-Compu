#!/bin/bash

# Check if the script is run as root
if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root (use sudo)."
    exit 1
fi

set -e
set -x

# Check if a valid mounting method argument is provided
if [ $# -ne 3 ]; then
    echo "Usage: $0 <mounting_method> <mountpoint> <image_filepath>"
    exit 1
fi
MOUNT_METHOD="$1"
MOUNT_POINT="$2"
IMAGE_FILEPATH="$3"

# call the unmount script
bash unmount.sh "$MOUNT_POINT"

sleep 1

# Attempt to mount with the chosen method
if [ "$MOUNT_METHOD" = "fat-fuse" ]; then
    ./fat-fuse "./$IMAGE_FILEPATH" "./$MOUNT_POINT"
    echo "Mounted with fat-fuse"
elif [ "$MOUNT_METHOD" = "native" ]; then
    if sudo mount -t vfat -o umask=000 "$IMAGE_FILEPATH" "$MOUNT_POINT"; then
        sleep 1
        echo "Mounted with mount"
        exit 0
    else
        echo "Failed to mount with mount"
        exit 1
    fi
else
    echo "Invalid mounting method. Use 'fat-fuse' or 'native'."
    exit 1
fi
