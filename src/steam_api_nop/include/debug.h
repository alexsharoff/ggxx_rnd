#pragma once

#include <Windows.h>

#ifdef STEAM_API_DBGPRINT
#define PRINT_FUNC() OutputDebugStringA(__FUNCTION__)
#else
#define PRINT_FUNC()
#endif