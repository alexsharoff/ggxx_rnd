#include "util.h"

#include <Windows.h>

#include <io.h>


void show_message_box(const wchar_t* message, bool error)
{
    UINT type = error ? MB_ICONWARNING : MB_ICONINFORMATION;
    type |= MB_TASKMODAL | MB_TOPMOST;
    ::MessageBoxW(NULL, message, L"GGXXACPR", type);
}

// Implementation taken from Chromium (process/launch_win.cc)
void attach_console(bool create_console_if_not_found)
{
    // Don't change anything if stdout or stderr already point to a
    // valid stream.
    //
    // If we are running under Visual Studio Code, stdout and stderr
    // will be pipe handles. In that case, we don't want to open CONOUT$,
    // because its output likely does not go anywhere.
    //
    // We don't use GetStdHandle() to check stdout/stderr here because
    // it can return dangling IDs of handles that were never inherited
    // by this process.  These IDs could have been reused by the time
    // this function is called.  The CRT checks the validity of
    // stdout/stderr on startup (before the handle IDs can be reused).
    // _fileno(stdout) will return -2 (_NO_CONSOLE_FILENO) if stdout was
    // invalid.
    if (::_fileno(stdout) >= 0 || ::_fileno(stderr) >= 0)
    {
        intptr_t stdout_handle = ::_get_osfhandle(_fileno(stdout));
        intptr_t stderr_handle = ::_get_osfhandle(_fileno(stderr));
        if (stdout_handle >= 0 || stderr_handle >= 0)
            return;
    }

    if (!::AttachConsole(ATTACH_PARENT_PROCESS))
    {
        unsigned int result = ::GetLastError();
        // Was probably already attached.
        if (result == ERROR_ACCESS_DENIED)
            return;
        // Don't bother creating a new console for each child process if the
        // parent process is invalid (eg: crashed).
        if (result == ERROR_GEN_FAILURE)
            return;
        if (create_console_if_not_found)
        {
            // Make a new console if attaching to parent fails with any other error.
            // It should be ERROR_INVALID_HANDLE at this point, which means the
            // browser was likely not started from a console.
            ::AllocConsole();
        }
        else
        {
            return;
        }
    }

    // Arbitrary byte count to use when buffering output lines.  More
    // means potential waste, less means more risk of interleaved
    // log-lines in output.
    enum
    {
        kOutputBufferSize = 64 * 1024
    };

    FILE* unused;
    if (::freopen_s(&unused, "CONOUT$", "w", stdout))
    {
        ::setvbuf(stdout, nullptr, _IOLBF, kOutputBufferSize);
        // Overwrite FD 1 for the benefit of any code that uses this FD
        // directly.  This is safe because the CRT allocates FDs 0, 1 and
        // 2 at startup even if they don't have valid underlying Windows
        // handles.  This means we won't be overwriting an FD created by
        // _open() after startup.
        ::_dup2(_fileno(stdout), 1);
    }
    if (::freopen_s(&unused, "CONOUT$", "w", stderr))
    {
        ::setvbuf(stderr, nullptr, _IOLBF, kOutputBufferSize);
        ::_dup2(_fileno(stderr), 2);
    }

    // Fix all cout, wcout, cin, wcin, cerr, wcerr, clog and wclog.
    std::ios::sync_with_stdio();
}
