#!/bin/bash

# Define the submodule directory and repository URL
# SUBMODULE_DIR="external/tinyobjloader"
# SUBMODULE_DIR="syclImplementation/external"
# REPO_URL="https://github.com/tinyobjloader/tinyobjloader.git"

# # Create the external directory if it does not exist
# mkdir -p syclImplementation/external

# # Check if the submodule directory already exists
# if [ -d "$SUBMODULE_DIR/.git" ]; then
#     echo "Updating existing tinyobjloader submodule..."
#     cd "$SUBMODULE_DIR" || exit
#     git pull origin master
#     cd - > /dev/null
# else
#     echo "Cloning tinyobjloader repository..."
#     git clone "$REPO_URL" "$SUBMODULE_DIR"
# fi

# Verify the result
# if [ -d "$SUBMODULE_DIR" ]; then
#     echo "tinyobjloader is set up successfully in $SUBMODULE_DIR"
# else
#     echo "Error: tinyobjloader failed to set up."
#     exit 1
# fi

SUBMODULE_DIR="syclImplementation/external/tinyusdz"
REPO_URL="https://github.com/lighttransport/tinyusdz.git"

mkdir -p syclImplementation/external

if [ -d "$SUBMODULE_DIR/.git" ]; then
    echo "Updating existing tinyusdz repository..."
    cd "$SUBMODULE_DIR" || exit 1
    # detect current remote default (usually 'release')
    DEFAULT_REF=$(git ls-remote --symref origin HEAD | awk -F'[: ]+' '/^ref:/ {print $3}')
    git fetch --tags origin
    git checkout "${DEFAULT_REF:-release}" || true
    git pull --ff-only origin "${DEFAULT_REF:-release}" || true
    cd - > /dev/null || exit 1
else
    echo "Cloning tinyusdz repository..."
    DEFAULT_REF=$(git ls-remote --symref "$REPO_URL" HEAD | awk -F'[: ]+' '/^ref:/ {print $3}')
    git clone --depth 1 -b "${DEFAULT_REF:-release}" "$REPO_URL" "$SUBMODULE_DIR"
fi

# Success check (and sanity: header should be under src/)
if [ -d "$SUBMODULE_DIR" ] && [ -f "$SUBMODULE_DIR/src/tinyusdz.hh" ]; then
    echo "tinyusdz is set up successfully in $SUBMODULE_DIR"
else
    echo "Error: tinyusdz failed to set up (tinyusdz.hh not found)."
    exit 1
fi




#!/usr/bin/env bash
# SUBMODULE_DIR="external/tinyusdz"
# REPO_URL="https://github.com/lighttransport/tinyusdz.git"

# mkdir -p external

# if [ -d "$SUBMODULE_DIR/.git" ]; then
#   echo "Updating existing tinyusdz repository..."
#   cd "$SUBMODULE_DIR" || exit 1
#   DEFAULT_REF=$(git ls-remote --symref origin HEAD | awk -F'[: ]+' '/^ref:/ {print $3}')
#   git fetch --tags origin
#   git checkout "${DEFAULT_REF:-release}" || true
#   git pull --ff-only origin "${DEFAULT_REF:-release}" || true
#   cd - >/dev/null || exit 1
# else
#   echo "Cloning tinyusdz repository..."
#   DEFAULT_REF=$(git ls-remote --symref "$REPO_URL" HEAD | awk -F'[: ]+' '/^ref:/ {print $3}')
#   git clone --depth 1 -b "${DEFAULT_REF:-release}" "$REPO_URL" "$SUBMODULE_DIR"
# fi

# # Ensure header is present where we expect it
# if [ -d "$SUBMODULE_DIR" ] && [ -f "$SUBMODULE_DIR/src/tinyusdz.hh" ]; then
#   echo "tinyusdz is set up successfully in $SUBMODULE_DIR"
# else
#   echo "Error: tinyusdz failed to set up (tinyusdz.hh not found)."
#   exit 1
# fi


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


# ─── Rust Environment and Build ─────────────────────────────────────
echo "Setting Rust build output to ./target ..."
# export CARGO_TARGET_DIR=./target

# echo "Compiling Rust project..."
# cargo build --release

# if [ -f "Cargo.toml" ]; then
#     cargo build --release
#     if [ $? -eq 0 ]; then
#         echo "✅ Rust build completed successfully."
#     else
#         echo "❌ Rust build failed!"
#         exit 1
#     fi
# else
#     echo "❌ Cargo.toml not found in current directory."
#     exit 1
# fi

MANIFEST="./rustLib/Cargo.toml"

export CARGO_TARGET_DIR="./rustLib/target"
if [ -f "$MANIFEST" ]; then
  echo "Building Rust crate (Release) from $MANIFEST ..."
  cargo build --release --manifest-path "$MANIFEST"
  echo "✅ Rust build done. Artifacts: ./target/release"
else
  echo "ℹ️ ./rustLib/Cargo.toml not found; skipping Rust build."
fi


# ─── Set Permissions ────────────────────────────────────────────────────
chmod +x TestGenerator.sh

if [ -f "./syclImplementation/build.sh" ]; then
    chmod +x ./syclImplementation/build.sh
else
    echo "⚠️ ./syclImplementation/build.sh not found. Skipping permission update."
fi

echo "🎉 Setup complete!"