#!/bin/bash

# --- Configuration ---
KERNEL_BIN="kernel.bin"
APPS_DIR="apps"

# Normalize app directory
if [ ! -d "$APPS_DIR" ] && [ -d "app" ]; then APPS_DIR="app"; fi

# Safety Checks
if [ ! -f "$KERNEL_BIN" ]; then
    echo "WARNING: '$KERNEL_BIN' not found."
    KERNEL_EXISTS=false
else
    KERNEL_EXISTS=true
fi

# List Disks
echo "=========================================="
echo " AVAILABLE EXTERNAL DISKS"
echo "=========================================="
diskutil list external
echo "=========================================="
echo ""

# Select Disk
echo "Enter the disk identifier (e.g., disk4):"
read -p "> " DISK_ID

if [ -z "$DISK_ID" ]; then echo "NO DISK SPECIFIED."; exit 1; fi
if [ "$DISK_ID" == "disk0" ]; then echo "SAFETY LOCK: disk0 is system."; exit 1; fi

# Mode Selection
echo ""
if [ "$KERNEL_EXISTS" = true ]; then
    read -p "[1] Flash Kernel (Sector 1)? [y/N]: " DO_FLASH
else
    DO_FLASH="n"
fi

if [ -d "$APPS_DIR" ]; then
    read -p "[2] Install Apps (Copy *.bin)? [y/N]: " DO_APPS
else
    DO_APPS="n"
fi

if [[ "$DO_FLASH" != "y" && "$DO_APPS" != "y" ]]; then exit 0; fi

# Execution

# --- FLASH KERNEL ---
if [[ "$DO_FLASH" == "y" || "$DO_FLASH" == "Y" ]]; then
    echo "------------------------------------------"
    echo "Unmounting..."
    diskutil unmountDisk /dev/$DISK_ID
    
    echo "Flashing Kernel (sudo required)..."
    sudo dd if=$KERNEL_BIN of=/dev/${DISK_ID/disk/rdisk} bs=512 seek=1
    if [ $? -ne 0 ]; then
        echo "FLASH FAILED. dd returned error."
        exit 1
    fi
    echo "Kernel Flashed Successfully."
fi

# --- INSTALL APPS ---
if [[ "$DO_APPS" == "y" || "$DO_APPS" == "Y" ]]; then
    echo "------------------------------------------"
    echo "Mounting..."
    diskutil mountDisk /dev/$DISK_ID > /dev/null
    
    PARTITION_ID="${DISK_ID}s1"
    MOUNT_POINT=$(diskutil info /dev/$PARTITION_ID | grep "Mount Point" | cut -d: -f2 | xargs)
    
    if [ -z "$MOUNT_POINT" ] || [ "$MOUNT_POINT" == "Not mounted" ]; then
        echo "Error: Could not mount partition $PARTITION_ID"
    else
        echo "Target: $MOUNT_POINT"
        
        # Clean Source Directory first
        echo "Cleaning source directory ($APPS_DIR)..."
        dot_clean "$APPS_DIR"

        # Immunize Destination
        echo "Immunizing SD Card..."
        mdutil -i off "$MOUNT_POINT" > /dev/null 2>&1
        touch "$MOUNT_POINT/.metadata_never_index"
        touch "$MOUNT_POINT/.Trashes"
        
        # Install Apps
        echo "Installing apps..."
        for f in $APPS_DIR/*.bin; do
            [ -e "$f" ] || continue
            # -X strips resource forks during copy
            cp -X "$f" "$MOUNT_POINT/"
            echo "   + $(basename "$f")"
        done

        # NUCLEAR CLEANUP (The Final Scrub)
        echo "Nuking Apple Droppings..."
        # Find and delete all files starting with ._ recursively
        find "$MOUNT_POINT" -name '._*' -delete
        # Delete specific folders
        rm -rf "$MOUNT_POINT/.Spotlight-V100" 2>/dev/null || true
        rm -rf "$MOUNT_POINT/.fseventsd" 2>/dev/null || true
        rm -rf "$MOUNT_POINT/.Trashes" 2>/dev/null || true
        
        sync
        echo "Apps Installed."
    fi
fi

# Cleanup
echo "------------------------------------------"
echo "Ejecting..."
diskutil eject /dev/$DISK_ID
echo "Done!"
