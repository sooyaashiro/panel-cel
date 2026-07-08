// panel-cel: launcher central (tema lila lavanda + fondo oscuro) que reune todos los
// lanzadores del celular en una sola interfaz, estilo QtScrcpy pero con mas estilo.
// Ventana sin bordes, esquinas redondeadas, botones dibujados a mano (owner-draw) con hover.
//
// - scrcpy (D3D11 / GPU / CPU): lanza el .exe con sus flags (replica los .bat, sin consola).
// - Herramientas (gnirehtet / TikTok): abre los .bat copiados en panel-cel\tools.
// - Volumen: abre el control de volumen y alterna "Iniciar con Windows".
//
// Icono: si existe panel-cel\icon.ico se usa (ventana + cabecera); si no, placeholder lavanda.
// Compilar:  .\build.ps1

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <tuple>

// ---------- Paleta (lila lavanda sobre oscuro) ----------
static const COLORREF BG          = RGB(28, 25, 43);
static const COLORREF HEADER_BG   = RGB(36, 32, 54);
static const COLORREF CARD        = RGB(45, 40, 66);
static const COLORREF CARD_HOVER  = RGB(64, 56, 96);
static const COLORREF STRIPE      = RGB(183, 157, 224);
static const COLORREF STRIPE_HOV  = RGB(208, 184, 255);
static const COLORREF ACCENT      = RGB(196, 170, 240);
static const COLORREF TXT         = RGB(237, 233, 247);
static const COLORREF SUB         = RGB(166, 157, 190);
static const COLORREF CLOSE_HOV   = RGB(206, 90, 104);

static const wchar_t* CLASS_NAME = L"PanelCelWnd";
static const wchar_t* RUN_KEY    = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const wchar_t* VOL_RUNVAL = L"PhoneVolume";

// Rutas
static const std::wstring D3D11_DIR = L"C:\\Users\\Rize\\scrcpy-d3d11";
static const std::wstring D3D11_EXE = L"C:\\Users\\Rize\\scrcpy-d3d11\\main.exe";
static const std::wstring GPU_DIR   = L"C:\\Users\\Rize\\scrcpy-gpu\\native";
static const std::wstring GPU_MAIN  = L"C:\\Users\\Rize\\scrcpy-gpu\\native\\main.exe";
static const std::wstring GPU_NVDEC = L"C:\\Users\\Rize\\scrcpy-gpu\\native\\main_gpu.exe";
static const std::wstring VOL_DIR   = L"C:\\Users\\Rize\\phone-volume";
static const std::wstring VOL_EXE   = L"C:\\Users\\Rize\\phone-volume\\main.exe";

static const int WIN_W = 600;
static const int HH    = 58;   // alto de la cabecera

// ---------- Estado ----------
struct Btn { RECT r; int id; std::wstring title; std::wstring sub; bool toggle; };
static std::vector<Btn> g_btns;
static std::vector<std::pair<RECT, std::wstring>> g_labels;
static int   g_hover = 0;
static bool  g_hovClose = false, g_hovMin = false;
static int   g_winH = 700;
static HICON g_icon = nullptr;
static HFONT g_fTitle, g_fGroup, g_fBtn, g_fSub, g_fIco;
static RECT  g_rClose, g_rMin;

// ---------- Autostart del control de volumen ----------
static bool volAutostartOn()
{
    HKEY k; if (RegOpenKeyExW(HKEY_CURRENT_USER, RUN_KEY, 0, KEY_READ, &k) != ERROR_SUCCESS) return false;
    bool has = RegQueryValueExW(k, VOL_RUNVAL, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS;
    RegCloseKey(k);
    return has;
}
static void setVolAutostart(bool on)
{
    HKEY k; if (RegOpenKeyExW(HKEY_CURRENT_USER, RUN_KEY, 0, KEY_SET_VALUE, &k) != ERROR_SUCCESS) return;
    if (on) {
        std::wstring q = L"\"" + VOL_EXE + L"\"";
        RegSetValueExW(k, VOL_RUNVAL, 0, REG_SZ, (const BYTE*)q.c_str(), (DWORD)((q.size()+1)*sizeof(wchar_t)));
    } else RegDeleteValueW(k, VOL_RUNVAL);
    RegCloseKey(k);
}

// ---------- Lanzar ----------
static void launchExe(const std::wstring& exe, const std::wstring& args, const std::wstring& cwd,
                      DWORD prio = NORMAL_PRIORITY_CLASS, DWORD_PTR aff = 0)
{
    std::wstring cl = L"\"" + exe + L"\"";
    if (!args.empty()) cl += L" " + args;
    std::vector<wchar_t> buf(cl.begin(), cl.end()); buf.push_back(0);
    STARTUPINFOW si{}; si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    if (CreateProcessW(nullptr, buf.data(), nullptr, nullptr, FALSE,
                       prio | CREATE_SUSPENDED, nullptr, cwd.c_str(), &si, &pi)) {
        if (aff) SetProcessAffinityMask(pi.hProcess, aff);
        ResumeThread(pi.hThread);
        CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
    }
}

static void runAction(HWND h, int id)
{
    switch (id) {
    case 1:  launchExe(D3D11_EXE, L"screenoff",            D3D11_DIR); break;
    case 2:  launchExe(D3D11_EXE, L"screenoff hd",         D3D11_DIR); break;
    case 3:  launchExe(D3D11_EXE, L"screenoff noaudio",    D3D11_DIR); break;
    case 6:  launchExe(GPU_MAIN,  L"screenoff",            GPU_DIR);   break;
    case 7:  launchExe(GPU_MAIN,  L"noaudio screenoff",    GPU_DIR);   break;
    case 4:  launchExe(GPU_NVDEC, L"noaudio screenoff",    GPU_DIR, BELOW_NORMAL_PRIORITY_CLASS); break;
    case 5:  launchExe(GPU_MAIN,  L"lite noaudio screenoff", GPU_DIR, BELOW_NORMAL_PRIORITY_CLASS, 0x30); break;
    case 10: launchExe(VOL_EXE, L"", VOL_DIR); break;
    case 11: setVolAutostart(!volAutostartOn()); InvalidateRect(h, nullptr, FALSE); break;
    }
}

// ---------- Layout ----------
static int layout()
{
    g_btns.clear(); g_labels.clear();
    const int margin = 20, gap = 12, cardH = 60;
    const int cardW = (WIN_W - margin * 2 - gap) / 2;
    int y = HH + 14;

    auto group = [&](const wchar_t* name,
                     std::vector<std::tuple<int, const wchar_t*, const wchar_t*, bool>> items) {
        g_labels.push_back({ { margin, y, WIN_W - margin, y + 22 }, name });
        y += 30;
        for (size_t i = 0; i < items.size(); ++i) {
            int col = (int)(i % 2);
            int x = margin + col * (cardW + gap);
            Btn b;
            b.r = { x, y, x + cardW, y + cardH };
            b.id = std::get<0>(items[i]);
            b.title = std::get<1>(items[i]);
            b.sub = std::get<2>(items[i]);
            b.toggle = std::get<3>(items[i]);
            g_btns.push_back(b);
            if (col == 1 || i + 1 == items.size()) y += cardH + gap;
        }
        y += 6;
    };

    group(L"ESPEJO DEL CELULAR  ·  scrcpy", {
        { 1, L"D3D11 · 1080 120fps", L"Audio en PC · zero-copy GPU", false },
        { 2, L"D3D11 · HD 1440",     L"Nitidez max · video y lectura", false },
        { 3, L"D3D11 · sin audio",   L"Sonido en el cel · mas fluido", false },
        { 6, L"CPU · panel apagado", L"Decode en CPU · fluidez pura", false },
        { 7, L"CPU · sin audio",     L"CPU · sonido en el cel", false },
        { 4, L"GPU/NVDEC · LoL",     L"Decode en GPU · CPU libre", false },
        { 5, L"Modo juego (lite)",   L"30fps · /low /affinity para LoL", false },
    });
    group(L"VOLUMEN DEL CELULAR", {
        { 10, L"Abrir control de volumen", L"Icono lavanda en la bandeja", false },
        { 11, L"Iniciar con Windows",      L"", true },
    });
    return y + 14;
}

// ---------- Dibujo ----------
static void roundFill(HDC dc, RECT r, int rad, COLORREF c)
{
    HBRUSH b = CreateSolidBrush(c);
    HGDIOBJ ob = SelectObject(dc, b);
    HGDIOBJ op = SelectObject(dc, GetStockObject(NULL_PEN));
    RoundRect(dc, r.left, r.top, r.right, r.bottom, rad, rad);
    SelectObject(dc, op); SelectObject(dc, ob);
    DeleteObject(b);
}

static void drawCard(HDC dc, const Btn& b)
{
    bool hov = (b.id == g_hover);
    bool on  = b.toggle && volAutostartOn();
    roundFill(dc, b.r, 12, hov ? CARD_HOVER : CARD);

    RECT sr{ b.r.left + 9, b.r.top + 11, b.r.left + 14, b.r.bottom - 11 };
    roundFill(dc, sr, 3, hov ? STRIPE_HOV : STRIPE);

    SetBkMode(dc, TRANSPARENT);
    SelectObject(dc, g_fBtn);
    SetTextColor(dc, TXT);
    RECT tr{ b.r.left + 26, b.r.top + 9, b.r.right - 14, b.r.top + 33 };
    DrawTextW(dc, b.title.c_str(), -1, &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    std::wstring sub = b.toggle ? (on ? L"Activado" : L"Desactivado") : b.sub;
    SelectObject(dc, g_fSub);
    SetTextColor(dc, (b.toggle && on) ? ACCENT : SUB);
    RECT sr2{ b.r.left + 26, b.r.top + 31, b.r.right - 14, b.r.bottom - 8 };
    DrawTextW(dc, sub.c_str(), -1, &sr2, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    if (b.toggle) {   // indicador on/off a la derecha
        int cx = b.r.right - 22, cy = (b.r.top + b.r.bottom) / 2;
        HBRUSH dot = CreateSolidBrush(on ? ACCENT : RGB(80, 74, 104));
        HGDIOBJ od = SelectObject(dc, dot);
        HGDIOBJ op = SelectObject(dc, GetStockObject(NULL_PEN));
        Ellipse(dc, cx - 6, cy - 6, cx + 6, cy + 6);
        SelectObject(dc, op); SelectObject(dc, od); DeleteObject(dot);
    }
}

static void drawHeaderBtn(HDC dc, RECT r, bool hov, bool isClose)
{
    if (hov) roundFill(dc, r, 8, isClose ? CLOSE_HOV : CARD_HOVER);
    HPEN pen = CreatePen(PS_SOLID, 2, hov ? RGB(255, 255, 255) : SUB);
    HGDIOBJ op = SelectObject(dc, pen);
    int cx = (r.left + r.right) / 2, cy = (r.top + r.bottom) / 2;
    if (isClose) {
        MoveToEx(dc, cx - 5, cy - 5, nullptr); LineTo(dc, cx + 6, cy + 6);
        MoveToEx(dc, cx + 5, cy - 5, nullptr); LineTo(dc, cx - 6, cy + 6);
    } else {
        MoveToEx(dc, cx - 5, cy, nullptr); LineTo(dc, cx + 6, cy);
    }
    SelectObject(dc, op); DeleteObject(pen);
}

static void onPaint(HWND h)
{
    PAINTSTRUCT ps; HDC hdc = BeginPaint(h, &ps);
    RECT rc; GetClientRect(h, &rc);
    HDC dc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HGDIOBJ ob = SelectObject(dc, bmp);

    HBRUSH bg = CreateSolidBrush(BG); FillRect(dc, &rc, bg); DeleteObject(bg);
    RECT hr{ 0, 0, rc.right, HH };
    HBRUSH hb = CreateSolidBrush(HEADER_BG); FillRect(dc, &hr, hb); DeleteObject(hb);

    // icono (o placeholder lavanda)
    RECT ir{ 18, 13, 50, 45 };
    if (g_icon) DrawIconEx(dc, ir.left, ir.top, g_icon, 32, 32, 0, nullptr, DI_NORMAL);
    else roundFill(dc, ir, 8, STRIPE);

    SetBkMode(dc, TRANSPARENT);
    SelectObject(dc, g_fTitle); SetTextColor(dc, TXT);
    RECT ttr{ 60, 0, rc.right - 110, HH };
    DrawTextW(dc, L"Panel del Celular", -1, &ttr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    drawHeaderBtn(dc, g_rMin, g_hovMin, false);
    drawHeaderBtn(dc, g_rClose, g_hovClose, true);

    for (auto& lb : g_labels) {
        SelectObject(dc, g_fGroup); SetTextColor(dc, ACCENT);
        RECT r = lb.first;
        DrawTextW(dc, lb.second.c_str(), -1, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }
    for (auto& b : g_btns) drawCard(dc, b);

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, dc, 0, 0, SRCCOPY);
    SelectObject(dc, ob); DeleteObject(bmp); DeleteDC(dc);
    EndPaint(h, &ps);
}

static int hitCard(POINT p)
{
    for (auto& b : g_btns) if (PtInRect(&b.r, p)) return b.id;
    return 0;
}

static LRESULT CALLBACK WndProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_ERASEBKGND: return 1;
    case WM_PAINT: onPaint(h); return 0;

    case WM_MOUSEMOVE: {
        POINT p{ GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
        int nh = hitCard(p);
        bool nc = PtInRect(&g_rClose, p), nm = PtInRect(&g_rMin, p);
        if (nh != g_hover || nc != g_hovClose || nm != g_hovMin) {
            g_hover = nh; g_hovClose = nc; g_hovMin = nm;
            InvalidateRect(h, nullptr, FALSE);
        }
        TRACKMOUSEEVENT t{ sizeof(t), TME_LEAVE, h, 0 };
        TrackMouseEvent(&t);
        return 0;
    }
    case WM_MOUSELEAVE:
        g_hover = 0; g_hovClose = g_hovMin = false;
        InvalidateRect(h, nullptr, FALSE);
        return 0;

    case WM_SETCURSOR:
        if (LOWORD(lp) == HTCLIENT) {
            POINT p; GetCursorPos(&p); ScreenToClient(h, &p);
            if (hitCard(p) || PtInRect(&g_rClose, p) || PtInRect(&g_rMin, p)) {
                SetCursor(LoadCursor(nullptr, IDC_HAND));
                return TRUE;
            }
        }
        break;

    case WM_LBUTTONDOWN: {
        POINT p{ GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
        if (p.y < HH && !PtInRect(&g_rClose, p) && !PtInRect(&g_rMin, p)) {
            ReleaseCapture();
            SendMessageW(h, WM_NCLBUTTONDOWN, HTCAPTION, 0);   // arrastrar ventana
        }
        return 0;
    }
    case WM_LBUTTONUP: {
        POINT p{ GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
        if (PtInRect(&g_rClose, p)) { DestroyWindow(h); return 0; }
        if (PtInRect(&g_rMin, p))   { ShowWindow(h, SW_MINIMIZE); return 0; }
        int id = hitCard(p);
        if (id) runAction(h, id);
        return 0;
    }

    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(h, msg, wp, lp);
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int)
{
    // icono desde panel-cel\icon.ico si existe
    wchar_t mp[MAX_PATH]; GetModuleFileNameW(nullptr, mp, MAX_PATH);
    std::wstring dir = mp; dir = dir.substr(0, dir.find_last_of(L"\\/"));
    g_icon = (HICON)LoadImageW(nullptr, (dir + L"\\icon.ico").c_str(), IMAGE_ICON,
                               0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);

    g_fTitle = CreateFontW(26, 0, 0, 0, FW_BOLD,     0,0,0, DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY, FF_SWISS, L"Segoe UI");
    g_fGroup = CreateFontW(13, 0, 0, 0, FW_BOLD,     0,0,0, DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY, FF_SWISS, L"Segoe UI");
    g_fBtn   = CreateFontW(18, 0, 0, 0, FW_SEMIBOLD, 0,0,0, DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY, FF_SWISS, L"Segoe UI");
    g_fSub   = CreateFontW(14, 0, 0, 0, FW_NORMAL,   0,0,0, DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY, FF_SWISS, L"Segoe UI");
    g_fIco   = g_fTitle;

    g_winH = layout();
    g_rMin   = { WIN_W - 88, 14, WIN_W - 58, 44 };
    g_rClose = { WIN_W - 46, 14, WIN_W - 16, 44 };

    WNDCLASSW wc{};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(BG);
    wc.hIcon         = g_icon;
    RegisterClassW(&wc);

    int sw = GetSystemMetrics(SM_CXSCREEN), sh = GetSystemMetrics(SM_CYSCREEN);
    int x = (sw - WIN_W) / 2, y = (sh - g_winH) / 2;
    HWND h = CreateWindowExW(WS_EX_APPWINDOW, CLASS_NAME, L"Panel del Celular",
        WS_POPUP | WS_MINIMIZEBOX, x, y, WIN_W, g_winH, nullptr, nullptr, hInst, nullptr);
    SetWindowRgn(h, CreateRoundRectRgn(0, 0, WIN_W + 1, g_winH + 1, 18, 18), TRUE);
    if (g_icon) { SendMessageW(h, WM_SETICON, ICON_BIG, (LPARAM)g_icon);
                  SendMessageW(h, WM_SETICON, ICON_SMALL, (LPARAM)g_icon); }
    ShowWindow(h, SW_SHOW);
    UpdateWindow(h);

    MSG m;
    while (GetMessageW(&m, nullptr, 0, 0)) { TranslateMessage(&m); DispatchMessageW(&m); }
    return 0;
}
