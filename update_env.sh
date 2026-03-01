#!/bin/zsh

# Check if /Volumes/docker exists
if [ ! -d "/Volumes/docker" ]; then
    echo "Mounting network volume..."
    mount_smbfs //seehausen@192.168.0.10/docker /Volumes/docker
    if [ $? -ne 0 ]; then
        echo "Failed to mount network volume. Exiting."
        exit 1
    fi
else
    echo "/Volumes/docker already mounted."
fi

SRC_DIR="/Users/seehausen/Developer/neowx-material/skins/neowx-material"
DEST_DIR="/Volumes/docker/weewx/skins/neowx-material"


# Read current version from destination skin.conf
echo "Reading current version from destination skin.conf..."
CURRENT_VERSION=$(grep -E '^\s*version\s*=' "$DEST_DIR/skin.conf" | sed -E 's/^[[:space:]]*version[[:space:]]*=[[:space:]]*([0-9]+\.[0-9]+\.[0-9]+).*$/\1/' | head -1)

if [ -z "$CURRENT_VERSION" ]; then
    echo "Error: Could not read version from skin.conf"
    exit 1
fi

# Verify we got just the version number
if ! echo "$CURRENT_VERSION" | grep -qE '^[0-9]+\.[0-9]+\.[0-9]+$'; then
    echo "Error: Invalid version format: '$CURRENT_VERSION'"
    echo "Expected format: x.y.z"
    exit 1
fi

echo "Current version: $CURRENT_VERSION"

# Parse version components
MAJOR=$(echo "$CURRENT_VERSION" | cut -d. -f1)
MINOR=$(echo "$CURRENT_VERSION" | cut -d. -f2)
PATCH=$(echo "$CURRENT_VERSION" | cut -d. -f3)

# Increment patch version
NEW_PATCH=$((PATCH + 1))
NEW_VERSION="${MAJOR}.${MINOR}.${NEW_PATCH}"

echo "New version: $NEW_VERSION"

# Copy files, excluding node_modules
rsync -av --exclude 'node_modules' "$SRC_DIR/" "$DEST_DIR/"

# Copy bin/user directory
echo "Copying bin/user directory..."
rsync -av "/Users/seehausen/Developer/neowx-material/bin/user/" "/Volumes/docker/weewx/bin/user"

# Copy JS files to web directory
echo "Copying JS files to /Volumes/web/js..."
rsync -av "$SRC_DIR/js/" "/Volumes/web/js/"

# Run config_patcher.py
cd "$DEST_DIR"
python3 config_patcher.py skin.conf skin.conf.patch

# Update version in destination skin.conf
echo "Updating version to $NEW_VERSION in destination skin.conf..."

# Clean up any corrupted lines and update version in one command
sed -i.bak -E "s/^[[:space:]]*version[[:space:]]*=[[:space:]]*.*$/    version = $NEW_VERSION/" "$DEST_DIR/skin.conf"

# Verify the update
UPDATED_VERSION=$(grep -E '^\s*version\s*=' "$DEST_DIR/skin.conf" | sed -E 's/^[[:space:]]*version[[:space:]]*=[[:space:]]*([0-9]+\.[0-9]+\.[0-9]+).*$/\1/' | head -1)
if [ "$UPDATED_VERSION" = "$NEW_VERSION" ]; then
    echo "✓ Version updated successfully from $CURRENT_VERSION to $NEW_VERSION"
    rm -f "$DEST_DIR/skin.conf.bak"
else
    echo "✗ Error: Version update failed. Expected $NEW_VERSION but got $UPDATED_VERSION"
    echo "Backup saved at $DEST_DIR/skin.conf.bak"
    exit 1
fi
