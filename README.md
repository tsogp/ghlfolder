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
- **[libzip](https://libzip.org/)** (for extracting zip archives)

### **Install dependencies on Ubuntu/Debian Linux**

```sh
sudo apt install nlohmann-json3-dev libzip-dev libssl-dev libpsl-dev zipcmp zipmerge ziptool 
```

**[libcpr](https://github.com/libcpr/cpr), [pranav/argparse](https://github.com/p-ranav/argparse)** and **[compile-time-regular-expressions](https://github.com/hanickadot/compile-time-regular-expressions)** are not available in the package managers, so those have to be built from source.

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
[GitHub API quota](https://docs.github.com/en/rest/using-the-rest-api/rate-limits-for-the-rest-api?apiVersion=2022-11-28) 
is 60 requests per hour for unauthenticated users and 5000 requests per hour for authenticated users.

If you don't want to authenticate with GitHub tokens, consider using ```--from_zip``` option, which will download the
whole zipball for you and then extract the required folder for you.

```sh
# Here were are downloading a part of Linux Kernel.
# NOTE: if --from_zip is not used here, all quota for your IP address will be gone
ghlfolder https://github.com/torvalds/linux/tree/master/arch --from_zip
```

If the folder that you need is small, not using ```--from_zip``` will result into faster download 

```sh
ghlfolder https://github.com/boostorg/pfr/tree/develop/include/boost
```

If you want to clone a private repository that you have access to or increase your quota, use ```--token=<token>```
with your GitHub token

```sh
ghlfolder https://github.com/<your_name>/<private_repo_name>/<subfolder> --token=<token>
```

If you would like to the results to be saved to some specific folder, use ```--output_dir=<dir>```

```sh
ghlfolder https://github.com/boostorg/pfr/tree/develop/include/boost --output_dir=/home/user
```
