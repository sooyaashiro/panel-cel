# Panel del Celular

Panel del Celular is a small native Windows launcher for phone-mirroring workflows built around scrcpy.

It provides a single custom Win32 interface for launching local scrcpy profiles, including D3D11, CPU, GPU/NVDEC, audio/no-audio, and a lightweight game mode. It also includes a shortcut to a phone-volume helper and a toggle for starting that helper with Windows.

## Configuration

The launcher reads local paths from `panel-cel.ini` next to `main.exe`.

Start by copying the example file:

```powershell
Copy-Item .\panel-cel.example.ini .\panel-cel.ini
```

Then edit `panel-cel.ini` with your local scrcpy and phone-volume paths. The local `panel-cel.ini` file is intentionally ignored by Git.

## Features

- Native Win32/C++ launcher with no web runtime.
- Custom borderless UI with rounded corners and hover states.
- Launch profiles for D3D11, CPU, GPU/NVDEC, and lightweight game mode.
- Optional phone-volume helper launcher.
- Toggle for Windows startup through the current-user Run registry key.

## Requirements

- Windows 10 or newer.
- A C++17 compiler. The included build script expects `g++` from `w64devkit`.
- Local scrcpy wrapper executables matching the paths configured in `panel-cel.ini`.

## Build

From PowerShell:

```powershell
.\build.ps1
```

Debug build:

```powershell
.\build_debug.ps1
```

## Repository layout

```text
src/main.cpp        Main launcher source.
src/main_debug.cpp  Debug build with logging.
src/config.h        INI configuration loader.
panel-cel.example.ini Example local configuration.
build.ps1           Release build script.
build_debug.ps1     Debug build script.
```

## Roadmap

- Add screenshots and a short demo GIF.
- Add a first GitHub release with a signed or checksummed executable.
- Document profile setup for scrcpy users.

## License

MIT. See `LICENSE`.
