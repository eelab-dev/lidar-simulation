# LidarSimulation



## Project description 

The purpose of this project is to generate synthetic LiDAR depth image for machine learning training with sycl for the purpose of compibility and performence which doesn't require specified vendor hardware  

## Dependencies
This project depends on:
1. **tinyobjloader** - for processing `.obj` mesh files (included as a submodule)
2. **SYCL OneAPI Environment** - Required for compilation and execution

## Envirnment Installation Instructions
The tinyobjloader is automatically configured in the following script
```bash
# make the script excutable
chomd +x setUp.sh

# runing the script
./setUp.sh
```


## OneAPI envirnment Installation Instructions

### 1. Clone the Repository and Initialize Submodules
```bash
# Clone the repository
git clone --recursive <repository_url>

# If you already cloned but forgot to initialize submodules, run:
git submodule update --init --recursive
```

### 2. Install SYCL OneAPI Environment
Follow the official installation guide for SYCL OneAPI on NVIDIA GPUs provided by **Codeplay**:
[Get Started Guide - Codeplay](https://developer.codeplay.com/products/oneapi/nvidia/2025.0.0/guides/get-started-guide-nvidia)

Alternatively, follow these steps:
```bash
# Download and install Codeplay’s OneAPI package
wget https://developer.codeplay.com/download/oneapi/nvidia/2025.0.0/linux/codeplay-oneapi-2025.0.0.tar.gz

# Extract the package
tar -xvzf codeplay-oneapi-2025.0.0.tar.gz

# Move to installation directory (optional)
sudo mv codeplay-oneapi-2025.0.0 /opt/oneapi
```

### 3. Set Up the Environment
```bash
# Source the environment setup script
source /opt/oneapi/setvars.sh
```
## Building Simulation Executable

### 1. Enter the Executable Directory
Navigate to the `syclImplementation` directory:
```bash
# Change to the directory containing the executable build scripts
cd syclImplementation  
```

### 2. Set Up the Environment
Execute the `configure.sh` script to set up the environment:
```bash
# Ensure the script is executable
chmod +x configure.sh 

# Run the environment setup script
./configure.sh  
```

### 3. Build the Executable
Run the `build.sh` script to compile the simulation executable:
```bash
# Ensure the build script is executable
chmod +x build.sh 

# Execute the build script to compile the project
./build.sh  
```

## Data Generation Flow
The simulation is executed using a SYCL binary after it has been successfully built. It requires a .obj
file to describe the surrounding environment, including a detector that captures returning photons. 
The simulation generates a dataset for each captured photon, which includes the distance traveled, 
the object it interacted with, and the photon's direction and position upon hitting the detector. 
This information is later used for pixelization and histogram generation via corresponding Python
scripts.The results are stored as intermediate outputs, enabling the histograms to be regenerated 
when configuration or demand parameters change.

## Generate Data Set With Bash File

### Go Back to the Root Directory
Navigate back to the root directory of the project:
```bash
cd ..
```

### Build Python Virtual Environment
Create a Python virtual environment using `requirements.txt` or Conda:
```bash
# Using virtualenv
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt

# Alternatively, using Conda
conda create --name myenv --file requirements.txt
conda activate myenv
```

### Execute `TestGenerator.sh`
Run the script to generate the dataset:
```bash
chmod +x TestGenerator.sh  # Ensure the script is executable
./TestGenerator.sh  # Execute the script
```

### Customize Dataset Generation
In `TestGenerator.sh`, users can adjust:
- The number of depth images to generate
- The folder name and file name of the generated dataset

## Result
The generated dataset is located in the folder specified in `TestGenerator.sh`. The dataset consists of three subfolders:
1. **Incoming Photo Information**
2. **Histogram Information for Depth Image**
3. **Depth Image Itself**


## Troubleshooting
- Ensure your GPU supports SYCL OneAPI
- Check the installation paths if the environment setup script is not found
- If compilation fails, verify that all dependencies are correctly installed




