#!/bin/bash

# update_version.sh - Updates version in multiple files and builds the project
# Usage: ./update_version.sh [--patch-version|--minor-version|--version x.x.x|--help|-h]

# Function to show help
show_help() {
    echo "update_version.sh - Updates version in multiple files and builds the project"
    echo ""
    echo "Usage:"
    echo "  ./update_version.sh [OPTION]"
    echo ""
    echo "Options:"
    echo "  --patch-version     Increment patch version by 1 (x.x.X)"
    echo "  --minor-version     Increment minor version by 1 and set patch to 0 (x.X.0)"
    echo "  --version x.x.x     Set specific version"
    echo "  --help, -h          Show this help message"
    echo ""
    echo "If no option is provided, --patch-version is used by default."
    echo ""
    echo "Examples:"
    echo "  ./update_version.sh                    # Increment patch version"
    echo "  ./update_version.sh --patch-version    # Increment patch version"
    echo "  ./update_version.sh --minor-version    # Increment minor version"
    echo "  ./update_version.sh --version 2.0.0    # Set to specific version"
    echo ""
    echo "The script updates version in:"
    echo "  - skins/neowx-material/skin.conf"
    echo "  - install.py"
    echo "  - skins/neowx-material/package.json"
    echo ""
    echo "And then runs: npm install && yarn run build"
}

# Function to get current version from skin.conf
get_current_version() {
    if [ -f "skins/neowx-material/skin.conf" ]; then
        grep "^[[:space:]]*version = " skins/neowx-material/skin.conf | head -1 | sed 's/.*version = //' | tr -d '\r'
    else
        echo "Error: skins/neowx-material/skin.conf not found"
        exit 1
    fi
}

# Function to increment patch version
increment_patch() {
    local version="$1"
    local major=$(echo "$version" | cut -d. -f1)
    local minor=$(echo "$version" | cut -d. -f2)
    local patch=$(echo "$version" | cut -d. -f3)
    echo "$major.$minor.$((patch + 1))"
}

# Function to increment minor version
increment_minor() {
    local version="$1"
    local major=$(echo "$version" | cut -d. -f1)
    local minor=$(echo "$version" | cut -d. -f2)
    echo "$major.$((minor + 1)).0"
}

# Check for help flags
if [[ "$1" == "--help" || "$1" == "-h" ]]; then
    show_help
    exit 0
fi

# If FileToPatchOnly is provided, only patch version in that file and exit
if [[ "$1" == FileToPatchOnly ]]; then
    PATCH_FILE="$2"
    if [ -z "$PATCH_FILE" ]; then
        echo "Error: FileToPatchOnly requires a file path as second argument."
        exit 1
    fi
    if [ ! -f "$PATCH_FILE" ]; then
        echo "Error: File $PATCH_FILE not found."
        exit 1
    fi
    CURRENT_VERSION=$(grep "^[[:space:]]*version = " "$PATCH_FILE" | head -1 | sed 's/.*version = //' | tr -d '\r')
    if [ -z "$CURRENT_VERSION" ]; then
        echo "Error: Could not find version in $PATCH_FILE."
        exit 1
    fi
    MAJOR=$(echo "$CURRENT_VERSION" | cut -d. -f1)
    MINOR=$(echo "$CURRENT_VERSION" | cut -d. -f2)
    PATCH=$(echo "$CURRENT_VERSION" | cut -d. -f3)
    NEW_VERSION="$MAJOR.$MINOR.$((PATCH + 1))"
    echo "Patching $PATCH_FILE: $CURRENT_VERSION -> $NEW_VERSION"
    if [[ "$OSTYPE" == "darwin"* ]]; then
        sed -i '' "s/^[[:space:]]*version = .*/    version = $NEW_VERSION/" "$PATCH_FILE"
    else
        sed -i "s/^[[:space:]]*version = .*/    version = $NEW_VERSION/" "$PATCH_FILE"
    fi
    echo "✓ Updated $PATCH_FILE"
    exit 0
fi

# Check for multiple parameters (only one should be provided)
if [ $# -gt 2 ]; then
    echo "Error: Too many parameters provided"
    echo "Only one option is allowed at a time"
    echo "Use --help for usage information"
    exit 1
fi

# Check for conflicting parameters
if [ $# -eq 2 ] && [[ "$1" != "--version" ]]; then
    echo "Error: Invalid parameter combination"
    echo "Only --version accepts an additional argument"
    echo "Use --help for usage information"
    exit 1
fi

# Check for update_version_only parameter
UPDATE_VERSION_ONLY=false
if [[ "$1" == "update_version_only" ]]; then
    UPDATE_VERSION_ONLY=true
    shift
fi

# Determine the version to use
if [ $# -eq 0 ]; then
    # Default: increment patch version
    CURRENT_VERSION=$(get_current_version)
    VERSION=$(increment_patch "$CURRENT_VERSION")
    echo "No parameter provided, incrementing patch version from $CURRENT_VERSION to $VERSION"
elif [[ "$1" == "--patch-version" ]]; then
    CURRENT_VERSION=$(get_current_version)
    VERSION=$(increment_patch "$CURRENT_VERSION")
    echo "Incrementing patch version from $CURRENT_VERSION to $VERSION"
elif [[ "$1" == "--minor-version" ]]; then
    CURRENT_VERSION=$(get_current_version)
    VERSION=$(increment_minor "$CURRENT_VERSION")
    echo "Incrementing minor version from $CURRENT_VERSION to $VERSION"
elif [[ "$1" == "--version" ]]; then
    if [ $# -ne 2 ]; then
        echo "Error: --version requires a version number"
        echo "Example: ./update_version.sh --version 1.53.0"
        exit 1
    fi
    VERSION="$2"
    echo "Setting version to: $VERSION"
else
    echo "Error: Unknown parameter '$1'"
    echo "Use --help for usage information"
    exit 1
fi

# Validate version format (basic check for semantic versioning)
if [[ ! $VERSION =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "Error: Version '$VERSION' doesn't follow semantic versioning (x.y.z)"
    exit 1
fi

echo "Updating version to: $VERSION"

# Update skin.conf
echo "Updating skins/neowx-material/skin.conf..."
if [ -f "skins/neowx-material/skin.conf" ]; then
    if [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS
        sed -i '' "s/^[[:space:]]*version = .*/    version = $VERSION/" skins/neowx-material/skin.conf
    else
        # Linux and other Unix-like systems
        sed -i "s/^[[:space:]]*version = .*/    version = $VERSION/" skins/neowx-material/skin.conf
    fi
    echo "✓ Updated skin.conf"
else
    echo "✗ Error: skins/neowx-material/skin.conf not found"
    exit 1
fi

# Update install.py
echo "Updating install.py..."
if [ -f "install.py" ]; then
    if [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS
        sed -i '' "s/version=\"[^\"]*\"/version=\"$VERSION\"/" install.py
    else
        # Linux and other Unix-like systems
        sed -i "s/version=\"[^\"]*\"/version=\"$VERSION\"/" install.py
    fi
    echo "✓ Updated install.py"
else
    echo "✗ Error: install.py not found"
    exit 1
fi

# Update package.json
echo "Updating skins/neowx-material/package.json..."
if [ -f "skins/neowx-material/package.json" ]; then
    if [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS
        sed -i '' "s/\"version\": \"[^\"]*\"/\"version\": \"$VERSION\"/" skins/neowx-material/package.json
    else
        # Linux and other Unix-like systems
        sed -i "s/\"version\": \"[^\"]*\"/\"version\": \"$VERSION\"/" skins/neowx-material/package.json
    fi
    echo "✓ Updated package.json"
else
    echo "✗ Error: skins/neowx-material/package.json not found"
    exit 1
fi

# Change to the skins/neowx-material directory for running commands
cd skins/neowx-material || {
    echo "✗ Error: Could not change to skins/neowx-material directory"
    exit 1
}

# Run npm install
echo "Running npm install..."
if npm install; then
    echo "✓ npm install completed successfully"
else
    echo "✗ Error: npm install failed"
    exit 1
fi

# Run yarn run build
echo "Running yarn run build..."
if yarn run build; then
    echo "✓ yarn run build completed successfully"
else
    echo "✗ Error: yarn run build failed"
    exit 1
fi

echo ""
echo "🎉 Version update completed successfully!"
if [[ -n "$CURRENT_VERSION" ]]; then
    echo "Previous version: $CURRENT_VERSION"
fi
echo "New version: $VERSION"
echo "Updated files:"
echo "  - skins/neowx-material/skin.conf"
echo "  - install.py"
echo "  - skins/neowx-material/package.json"
echo ""
echo "Build commands executed:"
echo "  - npm install"
echo "  - yarn run build"

