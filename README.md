# **GitHub Directory Downloader**  

[![Watch the video](https://github.com/tsogp/ghlfolder/blob/main/media/thumbnail.png?raw=true)](https://github.com/tsogp/ghlfolder/blob/main/media/demo.mp4?raw=true)

A **multithreaded** command-line tool to download directories from GitHub repositories using the GitHub API.  
Currently, it **only supports Linux**.

## **Features**  
- Converts GitHub repository URLs into GitHub API requests  
- Recursively downloads files from directories  
- Displays a progress bar for each file  
- Supports multithreading for faster downloads  

---

## **Installation**  

### **Prerequisites**  
Make sure you have the following installed on your system:  
- **C++ compiler (g++)**  
- **CMake**  
- **libcurl** (for HTTP requests)  
- **nlohmann/json** (for JSON parsing)  

### **Build Instructions**  
```sh
git clone https://github.com/tsogp/ghlfolder.git
cd ghlfolder
mkdir build && cd build
cmake ..
make
```

### **Usage**
Just copy the GitHub folder from the browser's URL bar and run it the same way you run *git clone*
```sh
./ghlfolder "https://github.com/<author>/<repo>/tree/<branch>/<folder>"
```