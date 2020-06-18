
/**
 * Steamless Steam DRM Remover
 * (c) 2015 atom0s [atom0s@live.com]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 */

#include "Utils.h"

#include <string>
#include <time.h>

/**
 * namespace Console
 *
 * Contains helper functions for console related functionality.
 */
namespace Console
{
    template<class StreamType>
    void SetColor(StreamType& stream, const Console::Colors& c)
    {
        // Handle timestamps if it is requested..
        if (c == Console::Colors::Timestamp)
        {
            __time32_t rawtime;
            struct tm timeinfo;

            _time32(&rawtime);
            _localtime32_s(&timeinfo, &rawtime);

            char szTimestamp[1024] = { 0 };
            strftime(szTimestamp, 1024, "[%m/%d/%y %H:%M:%S] ", &timeinfo);

            stream << Console::Colors::LightYelllow << szTimestamp;
        }

        ::SetConsoleTextAttribute(::GetStdHandle(STD_OUTPUT_HANDLE), (short)c);
    }

    std::ostream& operator << (std::ostream& s, const Console::Colors& c)
    {
        SetColor(s, c);
        return s;
    }

    std::wostream& operator << (std::wostream& s, const Console::Colors& c)
    {
        SetColor(s, c);
        return s;
    }

    void output(const char* format, ...)
    {
        char buffer[1024] = { 0 };
        va_list args;
        va_start(args, format);
        vsprintf_s(buffer, format, args);
        va_end(args);

        std::cout << Console::Colors::White << buffer;
    }

    void output(Console::Colors c, const char* format, ...)
    {
        char buffer[1024] = { 0 };
        va_list args;
        va_start(args, format);
        vsprintf_s(buffer, format, args);
        va_end(args);

        std::cout << c << buffer;
    }

    void output(Console::Colors c, const wchar_t* format, ...)
    {
        wchar_t buffer[1024] = { 0 };
        va_list args;
        va_start(args, format);
        vswprintf_s(buffer, format, args);
        va_end(args);

        std::wcout << c << buffer;
    }

    RevertColorsOnExit::RevertColorsOnExit() : m_color(static_cast<WORD>(-1))
    {
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (!::GetConsoleScreenBufferInfo(::GetStdHandle(STD_OUTPUT_HANDLE), &info))
            return;
        m_color = info.wAttributes;
    }

    RevertColorsOnExit::~RevertColorsOnExit()
    {
        if (m_color != -1)
            ::SetConsoleTextAttribute(::GetStdHandle(STD_OUTPUT_HANDLE), m_color);
    }
}; // namespace Console

/**
 * namespace Utils
 *
 * Contains various helpful functions.
 */
namespace Utils
{
    void DumpHexData(unsigned char* data, unsigned int size)
    {
        char szHexLine[1024] = { 0 };
        char szAsciiLine[1024] = { 0 };

        for (size_t x = 0; x < size; x++)
        {
            auto ascii = (unsigned char)data[x];
            if (ascii < 32 || 126 < ascii)
                ascii = (unsigned char)'.';

            sprintf_s(szHexLine, 1024, "%s %02hx", szHexLine, (unsigned char)data[x]);
            sprintf_s(szAsciiLine, 1024, "%s%c", szAsciiLine, ascii);

            if (((x + 1) % 16) == 0)
            {
                std::cout << Console::Colors::LightPurple << szHexLine << "    " << Console::Colors::LightYelllow << szAsciiLine << std::endl;
                memset(&szHexLine, 0x00, 1024);
                memset(&szAsciiLine, 0x00, 1024);
            }
        }

        if (strlen(szHexLine) > 0)
            std::cout << Console::Colors::LightPurple << szHexLine << "    " << Console::Colors::LightYelllow << szAsciiLine << std::endl;
    }

    bool MaskCompare(const unsigned char* lpDataPtr, const unsigned char* lpPattern, const char* pszMask)
    {
        for (; *pszMask; ++pszMask, ++lpDataPtr, ++lpPattern)
        {
            if (*pszMask == 'x' && *lpDataPtr != *lpPattern)
                return false;
        }
        return (*pszMask) == NULL;
    }

    unsigned int FindPattern(const unsigned char* lpData, unsigned int size, const unsigned char* lpPattern, const char* pszMask)
    {
        for (size_t x = 0; x < size; x++)
        {
            if (MaskCompare(lpData + x, lpPattern, pszMask))
                return ((unsigned int)lpData + x);
        }
        return 0;
    }
};
