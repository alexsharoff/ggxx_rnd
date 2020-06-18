
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

#ifndef __UTILS_H_INCLUDED__
#define __UTILS_H_INCLUDED__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <Windows.h>
#include <iostream>

/**
 * namespace Console
 *
 * Contains helper functions for console related functionality.
 */
namespace Console
{
    /**
     * @brief Color enumeration for console output.
     */
    enum Colors
    {
        // Custom color to display a timestamp..
        Timestamp = 0,

        // Red colors..
        Red = FOREGROUND_RED,
        LightRed = FOREGROUND_RED | FOREGROUND_INTENSITY,

        // Green colors..
        Green = FOREGROUND_GREEN,
        LightGreen = FOREGROUND_GREEN | FOREGROUND_INTENSITY,

        // Blue colors..
        Blue = FOREGROUND_BLUE,
        LightBlue = FOREGROUND_BLUE | FOREGROUND_INTENSITY,

        // Cyan colors..
        Cyan = FOREGROUND_BLUE | FOREGROUND_GREEN,
        LightCyan = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY,

        // Yellow colors..
        Yellow = FOREGROUND_GREEN | FOREGROUND_RED,
        LightYelllow = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY,

        // Purple colors..
        Purple = FOREGROUND_BLUE | FOREGROUND_RED,
        LightPurple = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY,

        // White colors..
        Grey = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        White = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,

        // Common color codes..
        Debug = LightCyan,
        Error = LightRed,
        Info = White,
        Success = LightGreen,
        Warning = LightYelllow
    };

    /**
     * @brief Operater overload to handle custom color codes in an ostream.
     *
     * @param s             The stream to inject the color code into.
     * @param c             The color code to inject.
     *
     * @return The incoming stream object.
     */
    std::ostream& operator << (std::ostream& s, const Console::Colors& c);

    /**
     * @brief Outputs the given string to the console.
     *
     * @param format        The format of the message to output.
     * @param ...           The arguments to fill the format.
     */
    void output(const char* format, ...);

    /**
     * @brief Outputs the given string to the console with the given color.
     *
     * @param c             The color to print the message with.
     * @param format        The format of the message to output.
     * @param ...           The arguments to fill the format.
     */
    void output(Console::Colors c, const char* format, ...);

    void output(Console::Colors c, const wchar_t* format, ...);

    struct RevertColorsOnExit
    {
        RevertColorsOnExit();
        ~RevertColorsOnExit();
    private:
        WORD m_color;
    };
}; // namespace Console

/**
 * namespace Utils
 *
 * Contains various helpful functions.
 */
namespace Utils
{
    /**
     * @brief Dumps the given hex data to the console.
     *
     * @param data          The data to dump.
     * @param size          The size of the data to dump.
     */
    void DumpHexData(unsigned char* data, unsigned int size);

    /**
     * @brief Compares a pattern against a given memory pointer.
     *
     * @param lpDataPtr     The live data to compare with.
     * @param lpPattern     The pattern of bytes to compare with.
     * @param pszMask       The mask to compare against.
     *
     * @return True if pattern was found, false otherwise.
     */
    bool MaskCompare(const unsigned char* lpDataPtr, const unsigned char* lpPattern, const char* pszMask);

    /**
     * @brief Locates a signature of bytes using the given mask within the given module.
     *
     * @param lpData        The data to scan for the pattern within.
     * @param size          The size of the data to scan within.
     * @param lpPattern     The pattern of bytes to compare with.
     * @param pszMask       The mask to compare against.
     *
     * @return Start address of where the pattern was found, NULL otherwise.
     */
    unsigned int FindPattern(const unsigned char* lpData, unsigned int size, const unsigned char* lpPattern, const char* pszMask);
};

#endif // __UTILS_H_INCLUDED__
