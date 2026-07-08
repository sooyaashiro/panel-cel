#pragma once

#include <windows.h>

#include <string>

struct PanelConfig {
    std::wstring d3d11Dir;
    std::wstring d3d11Exe;
    std::wstring gpuDir;
    std::wstring gpuMainExe;
    std::wstring gpuNvdecExe;
    std::wstring volDir;
    std::wstring volExe;
};

inline std::wstring pathJoin(const std::wstring& base, const std::wstring& name)
{
    if (base.empty()) return name;
    wchar_t last = base.back();
    if (last == L'\\' || last == L'/') return base + name;
    return base + L"\\" + name;
}

inline std::wstring moduleDirectory()
{
    wchar_t modulePath[MAX_PATH];
    GetModuleFileNameW(nullptr, modulePath, MAX_PATH);

    std::wstring dir = modulePath;
    size_t pos = dir.find_last_of(L"\\/");
    if (pos == std::wstring::npos) return L".";
    return dir.substr(0, pos);
}

inline std::wstring readIniString(const std::wstring& iniPath,
                                  const wchar_t* section,
                                  const wchar_t* key,
                                  const std::wstring& fallback)
{
    wchar_t value[4096];
    DWORD len = GetPrivateProfileStringW(section, key, fallback.c_str(),
                                         value, (DWORD)(sizeof(value) / sizeof(value[0])),
                                         iniPath.c_str());
    return std::wstring(value, len);
}

inline PanelConfig loadPanelConfig(const std::wstring& appDir)
{
    const std::wstring iniPath = pathJoin(appDir, L"panel-cel.ini");
    const std::wstring toolsDir = pathJoin(appDir, L"tools");

    PanelConfig cfg;
    cfg.d3d11Dir = readIniString(iniPath, L"scrcpy", L"d3d11_dir",
                                 pathJoin(toolsDir, L"scrcpy-d3d11"));
    cfg.d3d11Exe = readIniString(iniPath, L"scrcpy", L"d3d11_exe",
                                 pathJoin(cfg.d3d11Dir, L"main.exe"));

    cfg.gpuDir = readIniString(iniPath, L"scrcpy", L"gpu_dir",
                               pathJoin(pathJoin(toolsDir, L"scrcpy-gpu"), L"native"));
    cfg.gpuMainExe = readIniString(iniPath, L"scrcpy", L"gpu_main_exe",
                                   pathJoin(cfg.gpuDir, L"main.exe"));
    cfg.gpuNvdecExe = readIniString(iniPath, L"scrcpy", L"gpu_nvdec_exe",
                                    pathJoin(cfg.gpuDir, L"main_gpu.exe"));

    cfg.volDir = readIniString(iniPath, L"volume", L"volume_dir",
                               pathJoin(toolsDir, L"phone-volume"));
    cfg.volExe = readIniString(iniPath, L"volume", L"volume_exe",
                               pathJoin(cfg.volDir, L"main.exe"));

    return cfg;
}
