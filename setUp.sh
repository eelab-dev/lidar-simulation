#!/bin/bash

# Define the submodule directory and repository URL
SUBMODULE_DIR="external/tinyobjloader"
REPO_URL="https://github.com/tinyobjloader/tinyobjloader.git"

# Create the external directory if it does not exist
mkdir -p external

# Check if the submodule directory already exists
if [ -d "$SUBMODULE_DIR/.git" ]; then
    echo "Updating existing tinyobjloader submodule..."
    cd "$SUBMODULE_DIR" || exit
    git pull origin master
    cd - > /dev/null
else
    echo "Cloning tinyobjloader repository..."
    git clone "$REPO_URL" "$SUBMODULE_DIR"
fi

# Verify the result
if [ -d "$SUBMODULE_DIR" ]; then
    echo "tinyobjloader is set up successfully in $SUBMODULE_DIR"
else
    echo "Error: tinyobjloader failed to set up."
    exit 1
fi
