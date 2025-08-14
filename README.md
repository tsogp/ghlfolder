# ghlfolder - **GitHub Directory Downloader**  

![ghlfolder_demo](https://github.com/user-attachments/assets/aecf2980-8a61-41af-8d63-7e8fa148b9f8)

A command-line tool to download directories from GitHub repositories using the GitHub API.

## **Preface**

I have always been frustrated when I had to download a subfolder from any GitHub repository. 
The only choice is to download the whole repository in a zipball and then manually locate the required folder. 
There are websites that do this (like [this one](https://github.com/download-directory/download-directory.github.io) from Federico Brigante, check out their great work!), 
but it still results into a zip archive which you have to manually extract. I wanted it to be as convenient as ```git clone```,
which is why I created **ghlfolder**.


## **Prerequisites**  

The project uses the following libraries:   
- **[pranav/argparse](https://github.com/p-ranav/argparse)** (for parsing command line arguments)
- **[libcpr](https://github.com/libcpr/cpr)** (C++ cURL wrapper for HTTP requests)  
- **[nlohmann/json](https://github.com/nlohmann/json)** (for JSON parsing)
- **[compile-time-regular-expressions](https://github.com/hanickadot/compile-time-regular-expressions)** (for compile time regular expressions)

### **Install dependencies on Ubuntu/Debian Linux**

```sh
sudo apt install nlohmann-json3-dev libzip-dev libssl-dev libpsl-dev zipcmp zipmerge ziptool 
```

**[pranav/argparse](https://github.com/p-ranav/argparse)** and **[compile-time-regular-expressions](https://github.com/hanickadot/compile-time-regular-expressions)** are not available in the package managers, so those have to be built from source.

```sh
# Build libcpr
git clone https://github.com/libcpr/cpr.git
cd cpr && mkdir build && cd build
cmake .. -DCPR_USE_SYSTEM_CURL=ON
cmake --build . --parallel
sudo cmake --install .

# Build argparse
git clone https://github.com/p-ranav/argparse.git
cd argparse
cmake -B build -S .
sudo cmake --install build

# Build CTRE
git clone https://github.com/hanickadot/compile-time-regular-expressions.git
cd compile-time-regular-expressions
cmake -B build -S .
sudo cmake --install build
```
### **Install dependencies on Arch Linux**

```sh
sudo pacman -Syu nlohmann-json cpr libzip
yay -S argparse ctre
```

### **Install dependencies on Windows**

Preferred way to install dependencies on Windows is **[vcpkg](https://github.com/microsoft/vcpkg)** package manager.

```
vcpkg install
```

## **Build Instructions**

### Linux

```sh
git clone https://github.com/tsogp/ghlfolder.git
cd ghlfolder
cmake -B build -S .
cmake --build build --parallel --config Release
sudo cmake --install build --config Release
```

### Windows with vckpg

```sh
git clone https://github.com/tsogp/ghlfolder.git
cd ghlfolder
cmake -B build -S . `
  -DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x64-windows `
  -G "Visual Studio 17 2022"
cmake --build build --parallel --config Release
cmake --install build --config Release
```

### **Usage**
Just copy the GitHub folder from the browser's URL bar and run it the same way you run *git clone*
```sh
./ghlfolder "https://github.com/<author>/<repo>/tree/<branch>/<folder>"
./ghlfolder "https://github.com/<author>/<repo>/tree/<branch>/<folder>" --output_dir=/home/user
```
