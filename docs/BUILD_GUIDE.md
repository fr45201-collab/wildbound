# Wildbound Build Guide

## Prerequisites
- **Operating System**: Windows 10/11 (64-bit)
- **Compiler**: Visual Studio 2022 or 2026 with Desktop development with C++ (MSVC 19.3x or 19.5x)
- **CMake**: Version 3.20 or newer
- **Git**: Installed and available in PATH

## Quick Build (Command Line)

### 1. Configure the Project
```powershell
cmake -B build -S .
```
Dependencies (`raylib`, `lua`, `sol2`, `imgui`, `rlimgui`) are fetched and configured automatically via CMake FetchContent.

### 2. Compile Debug Configuration
```powershell
cmake --build build --config Debug
```
The resulting executable will be placed at `build/Debug/wildbound.exe`.

### 3. Compile Release Configuration
```powershell
cmake --build build --config Release
```
The resulting executable will be placed at `build/Release/wildbound.exe`.

### 4. Run Automated Tests
```powershell
ctest --test-dir build -C Debug --output-on-failure
```

### 5. Launch the Game
```powershell
.\build\Debug\wildbound.exe
```
Or run the non-blocking smoke verification:
```powershell
.\build\Debug\wildbound.exe --test-run
```
