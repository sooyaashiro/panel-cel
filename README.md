# Panel del Celular

Panel del Celular is a small native Windows launcher for phone-mirroring workflows built around scrcpy.

It provides a single custom Win32 interface for launching local scrcpy profiles, including D3D11, CPU, GPU/NVDEC, audio/no-audio, and a lightweight game mode. It also includes a shortcut to a phone-volume helper and a toggle for starting that helper with Windows.

## Current status

This project is currently a personal Windows utility being prepared for public open-source release.

Important limitation: the current source uses local absolute paths such as `C:\Users\Rize\scrcpy-d3d11` and `C:\Users\Rize\phone-volume`. Before this is broadly reusable, those paths should be moved to a config file or auto-discovery logic.

## Features

- Native Win32/C++ launcher with no web runtime.
- Custom borderless UI with rounded corners and hover states.
- Launch profiles for D3D11, CPU, GPU/NVDEC, and lightweight game mode.
- Optional phone-volume helper launcher.
- Toggle for Windows startup through the current-user Run registry key.

## Requirements

- Windows 10 or newer.
- A C++17 compiler. The included build script expects `g++` from `w64devkit`.
- Local scrcpy wrapper executables matching the paths configured in `src/main.cpp`.

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
build.ps1           Release build script.
build_debug.ps1     Debug build script.
```

## Roadmap

- Replace hardcoded local paths with a config file.
- Add screenshots and a short demo GIF.
- Add a first GitHub release with a signed or checksummed executable.
- Document profile setup for scrcpy users.

## License

MIT. See `LICENSE`.
