#include "ui.h"

#include <Windows.h>


void show_message_box(const wchar_t* message, bool error)
{
    UINT type = error ? MB_ICONWARNING : MB_ICONINFORMATION;
    type |= MB_TASKMODAL | MB_TOPMOST;
    ::MessageBoxW(NULL, message, L"GGXXACPR", type);
}
