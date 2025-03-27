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

# Update package lists
echo "Updating package lists..."
sudo apt-get update

# Check if HDF5 development library is already installed
if dpkg -s libhdf5-dev &> /dev/null; then
    echo "libhdf5-dev is already installed. Skipping installation."
else
    # Install HDF5 development library
    echo "Installing HDF5 development library..."
    sudo apt-get install -y libhdf5-dev
fi

# Verify installation
echo "Checking HDF5 installation..."
if dpkg -l | grep -q libhdf5-dev; then
    echo "✅ HDF5 installed successfully!"
else
    echo "❌ HDF5 installation failed!"
    exit 1
fi

# ─── Python Virtual Environment Setup ────────────────────────────────

# Check if requirements.txt exists
if [ ! -f "requirements.txt" ]; then
    echo "Warning: requirements.txt not found. Skipping virtual environment setup."
    exit 0
fi

# Create a virtual environment if it doesn't exist
if [ ! -d ".venv" ]; then
    echo "Creating Python virtual environment in .venv..."
    python3 -m venv .venv
fi

# Activate the virtual environment and install requirements
echo "Activating virtual environment and installing dependencies..."
source .venv/bin/activate
pip install --upgrade pip
pip install -r requirements.txt