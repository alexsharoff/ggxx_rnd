#include "patches/load_libgg_dll.h"
#include "resource.h"

#include <PEFile.h>
#include <SteamStub32_Variant3.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <Windows.h>
#include <CommCtrl.h>
#include <commdlg.h>


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#define ID_INSTALL 1
#define ID_UNINSTALL 2

const UINT g_padding = 10;
HINSTANCE g_hInstance;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    WNDCLASS wc = {0};
    wc.lpszClassName = "libgg patcher";
    wc.hInstance = hInstance;
    wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wc.lpfnWndProc = WndProc;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    // TODO: create icon
    //wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDR_ICON));

    RegisterClass(&wc);
    CreateWindowEx(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        wc.lpszClassName, wc.lpszClassName,
        WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 1, 1,
        0, 0, hInstance, 0
    );

    g_hInstance = hInstance;

    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

void CreateTooltip(HWND parentHwnd, char* text)
{
    INITCOMMONCONTROLSEX iccex{};
    iccex.dwICC = ICC_WIN95_CLASSES;
    iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    InitCommonControlsEx(&iccex);

    auto tooltipHwnd = CreateWindowEx(
        WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
        0, 0, 0, 0, parentHwnd, NULL, NULL, NULL
    );

    SetWindowPos(
        tooltipHwnd, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
    );

    RECT rect;
    GetClientRect(parentHwnd, &rect);

    TOOLINFO ti{};
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_SUBCLASS;
    ti.hwnd = parentHwnd;
    ti.hinst = NULL;
    ti.uId = 0;
    ti.lpszText = text;
    ti.rect.left = rect.left;
    ti.rect.top = rect.top;
    ti.rect.right = rect.right;
    ti.rect.bottom = rect.bottom;

    SendMessage(tooltipHwnd, TTM_ADDTOOL, 0, (LPARAM)&ti);
}

std::wstring GetGGXXACPRPath(HWND hwnd)
{
    wchar_t path[MAX_PATH] = { 0 };

    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"GGXXACPR_Win.exe\0GGXXACPR_Win.exe\0\0";
    const wchar_t initial_dir[] = L"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Guilty Gear XX Accent Core Plus R";
    if (std::filesystem::is_directory(initial_dir))
        ofn.lpstrInitialDir = initial_dir;
    else
        ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_DONTADDTORECENT | OFN_NOREADONLYRETURN | OFN_HIDEREADONLY;

    if (!GetOpenFileNameW(&ofn))
        return L"";

    return path;
}

bool WriteLibggDll(HINSTANCE hInstance, const wchar_t* path)
{
    auto resource = FindResource(hInstance, MAKEINTRESOURCE(IDR_LIBGG_DLL), RT_RCDATA);
    if (!resource)
        return false;
    auto handle = LoadResource(NULL, resource);
    if (!handle)
        return false;
    auto data = (const char*)LockResource(handle);
    struct UnlockGuard
    {
        ~UnlockGuard()
        {
            UnlockResource(handle);
        }
        HGLOBAL handle;
    } guard { handle };
    auto size = SizeofResource(NULL, resource);
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs.is_open())
        return false;
    if (!ofs.write(data, size))
        return false;
    return true;
}

void ShowMessageBox(HWND owner, const wchar_t* caption, const wchar_t* message, bool error)
{
    UINT type = error ? MB_ICONWARNING : MB_ICONINFORMATION;
    type |= MB_TASKMODAL | MB_TOPMOST;
    MessageBoxW(owner, message, caption, type);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    HWND childHwnd;
    std::wstring ggxxacpr_path;
    switch(msg)
    {
        case WM_CREATE:
            childHwnd = CreateWindowW(
                L"Button", L"Install",
                WS_VISIBLE | WS_CHILD,
                g_padding, g_padding, 100, 40, hwnd,
                (HMENU)ID_INSTALL, NULL, NULL
            );
            CreateTooltip(childHwnd, "Backup GGXXACPR_Win.exe, apply patch, create libgg.dll");

            childHwnd = CreateWindowW(
                L"Button", L"Uninstall",
                WS_VISIBLE | WS_CHILD ,
                100 + g_padding * 2, g_padding, 100, 40, hwnd,
                (HMENU)ID_UNINSTALL, NULL, NULL
            );
            CreateTooltip(childHwnd, "Restore GGXXACPR_Win.exe, remove libgg.dll");

            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case ID_INSTALL:
                    ggxxacpr_path = GetGGXXACPRPath(hwnd);
                    if (!ggxxacpr_path.empty())
                    {
                        const auto caption = L"Install libgg";
                        const auto libgg_path = std::filesystem::path(ggxxacpr_path).parent_path() / "libgg.dll";
                        const auto backup_path = ggxxacpr_path + L".backup.exe";
                        if (std::filesystem::exists(libgg_path) || std::filesystem::exists(backup_path))
                        {
                            ShowMessageBox(hwnd, caption, L"Already installed.", true);
                            break;
                        }

                        if (!WriteLibggDll(g_hInstance, libgg_path.c_str()))
                        {
                            ShowMessageBox(hwnd, caption, L"Unable to create libgg.dll.", true);
                            break;
                        }

                        if (!std::filesystem::copy_file(ggxxacpr_path, backup_path))
                        {
                            ShowMessageBox(hwnd, caption, L"Unable to backup GGXXACPR_Win.exe.", true);
                            break;
                        }

                        PEFile pefile;
                        if (!pefile.Initialize(ggxxacpr_path))
                        {
                            ShowMessageBox(hwnd, caption, L"Unable to open GGXXACPR_Win.exe.", true);
                            break;
                        }

                        if (!SteamStub32Variant3::ProcessFileEx(&pefile, ggxxacpr_path.c_str(), false, false, true, false))
                        {
                            ShowMessageBox(hwnd, caption, L"Unable to decrypt GGXXACPR_Win.exe.", true);
                            break;
                        }

                        if (!apply_patch_to_file(ggxxacpr_path.c_str(), g_patch_load_libgg_dll))
                        {
                            ShowMessageBox(hwnd, caption, L"Unable to patch GGXXACPR_Win.exe.", true);
                            break;
                        }

                        if (!SteamStub32Variant3::ProcessFileEx(&pefile, ggxxacpr_path.c_str(), false, false, false, false))
                        {
                            ShowMessageBox(hwnd, caption, L"Unable to encrypt GGXXACPR_Win.exe.", true);
                            break;
                        }

                        ShowMessageBox(hwnd, caption, L"Success!", false);

                        PostQuitMessage(0);
                    }
                    break;
                case ID_UNINSTALL:
                    ggxxacpr_path = GetGGXXACPRPath(hwnd);
                    if (!ggxxacpr_path.empty())
                    {
                        const auto caption = L"Uninstall libgg";
                        const auto libgg_path = std::filesystem::path(ggxxacpr_path).parent_path() / "libgg.dll";
                        const auto backup_path = ggxxacpr_path + L".backup.exe";
                        if (std::filesystem::exists(backup_path))
                        {
                            if (!std::filesystem::copy_file(backup_path, ggxxacpr_path, std::filesystem::copy_options::overwrite_existing))
                            {
                                ShowMessageBox(hwnd, caption, L"Unable to restore GGXXACPR_Win.exe backup.", true);
                                break;
                            }
                            if (!std::filesystem::remove(backup_path))
                            {
                                ShowMessageBox(hwnd, caption, L"Unable to remove GGXXACPR_Win.exe backup.", true);
                                break;
                            }
                        }

                        if (std::filesystem::exists(libgg_path))
                        {
                            if (!std::filesystem::remove(libgg_path))
                            {
                                ShowMessageBox(hwnd, caption, L"Unable to remove libgg.dll.", true);
                                break;
                            }
                        }

                        ShowMessageBox(hwnd, caption, L"Success!", false);

                        PostQuitMessage(0);
                    }
                    break;
            }

            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_PARENTNOTIFY:
            // Autosize parent window
            WINDOWINFO windowsInfo;
            RECT parentRect, childRect;
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
                case WM_CREATE:
                    childHwnd = (HWND)lParam;
                    GetWindowInfo(childHwnd, &windowsInfo);
                    childRect = windowsInfo.rcWindow;
                    GetWindowInfo(hwnd, &windowsInfo);
                    parentRect = windowsInfo.rcWindow;
                    parentRect.right = std::max(
                        parentRect.right, (LONG)(childRect.right + windowsInfo.cxWindowBorders + g_padding)
                    );
                    parentRect.bottom = std::max(
                        parentRect.bottom, (LONG)(childRect.bottom + windowsInfo.cyWindowBorders + g_padding)
                    );
                    SetWindowPos(
                        hwnd, 0, 0, 0,
                        parentRect.right - parentRect.left,
                        parentRect.bottom - parentRect.top,
                        SWP_NOZORDER | SWP_NOMOVE
                    );
                    break;
            }
            break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
