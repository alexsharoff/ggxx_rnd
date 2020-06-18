
/**
 * Steamless Steam DRM Remover - SteamStub_Variant3.h
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

#ifndef __STEAMSTUB32VARIANT3_H_INCLUDED__
#define __STEAMSTUB32VARIANT3_H_INCLUDED__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "PEFile.h"

namespace SteamStub32Variant3
{
    /**
     * @brief The information block that holds the DRM information.
     */
    typedef struct tagSteamStub32Var3Header
    {
        unsigned int        XorKey;                     // The base XOR key, if defined, to unpack the file with.
        unsigned int        Signature;                  // 0xC0DEC0DE signature to validate this header is proper.
        unsigned int        ImageBase;                  // The base of the image that is protected.
        unsigned int        Unknown0000;                // Seems to always be 0.
        unsigned int        AddressOfEntryPoint;        // The entry point that is set from the DRM.
        unsigned int        BindSectionOffset;          // The starting offset to the bind section data. RVA(AddressOfEntryPoint - BindSectionOffset)
        unsigned int        Unknown0001;                // Unknown. Changes per-file. (Ranges have been 0x1000 -> 0x2000)
        unsigned int        OriginalEntryPoint;         // The original entry point of the binary before it was protected.
        unsigned int        Unknown0002;                // Seems to always be 0.
        unsigned int        PayloadSize;                // The size of the payload data.
        unsigned int        DRMPDLLOffset;              // The offset to the SteamDRMP.dll file.
        unsigned int        DRMPDLLSize;                // The size of the SteamDRMP.dll file.
        unsigned int        Unknown0003;
        unsigned int        Unknown0004;
        unsigned int        Unknown0005;
        unsigned int        Unknown0006;
        unsigned int        Unknown0007;                // Possible section alignment.
        unsigned int        TextSectionRawSize;         // The raw size of the text section.
        unsigned char       AES_Key[0x20];              // The AES encryption key.
        unsigned char       AES_IV[0x10];               // The AES encryption IV.
        unsigned char       TextSectionStolenData[0x10];// The first 16 bytes of the .text section stolen.
        unsigned int        EncryptionKeys[0x04];       // Encryption keys used for decrypting SteamDRMP.dll file.
        unsigned int        Unknown0008;
        unsigned int        Unknown0009;
        unsigned int        Unknown0010;
        unsigned int        Unknown0011;
        unsigned int        Unknown0012;
        unsigned int        Unknown0013;
        unsigned int        GetModuleHandleA_RVA;       // The RVA to GetModuleHandleA.
        unsigned int        GetModuleHandleW_RVA;       // The RVA to GetModuleHandleW.
        unsigned int        LoadLibraryA_RVA;           // The RVA to LoadLibraryA.
        unsigned int        LoadLibraryW_RVA;           // The RVA to LoadLibraryW.
        unsigned int        GetProcAddress_RVA;         // The RVA to GetProcAddress.
        unsigned int        Unknown0014;
        unsigned int        Unknown0015;
        unsigned int        Unknown0016;
    } SteamStub32Var3Header;

    /**
     * @brief Xor decrypts the given data starting with the given key, if any.
     *
     * @param data          The data to xor.
     * @param size          The size of the data to xor.
     * @param key           The starting key to xor with.
     *
     * @note    If no key is given (0) then the first key is read from the first
     *          4 bytes inside of the data given.
     */
    unsigned int SteamXor(unsigned char* data, unsigned int size, unsigned int key = 0);

    /**
     * @brief The second pass of decryption for the SteamDRMP.dll file.
     *
     * @param res           The result value buffer to write our returns to.
     * @param keys          The keys used for the decryption.
     * @param v1            The first value to decrypt from.
     * @param v2            The second value to decrypt from.
     * @param n             The number of passes to crypt the data with.
     *
     * @note    The encryption method here is known as XTEA.
     */
    void SteamDrmpDecryptPass2(unsigned int res[], unsigned int* keys, unsigned int v1, unsigned int v2, unsigned int n = 32);

    /**
     * @brief The first pass of the decryption for the SteamDRMP.dll file.
     *
     * @param data          The data to decrypt.
     * @param size          The size of the data to decrypt.
     * @param keys          The keys used for the decryption.
     *
     * @note    The encryption method here is known as XTEA. It is modded to include
     *          some basic xor'ing.
     */
    void SteamDrmpDecryptPass1(unsigned char* data, unsigned int size, unsigned int* keys);

    /**
     * @brief Processes the given file for unpacking.
     *
     * @param file            The file being unpacked.
     * @param unpackedPath    Output file path.
     * @param dumpSteamDrmp   Dump SteamDRMP.dll in current directory or not.
     *
     * @return True on success, false otherwise.
     */
    bool ProcessFile(PEFile* file, const wchar_t* unpackedPath);

    bool ProcessFileEx(
        PEFile* file,
        const wchar_t* unpackedPath,
        bool dumpSteamDrmp = true,
        bool removeBindSection = true,
        bool decryptTextSection = true,
        bool verbose = true
    );

}; // namespace SteamStub32Variant3

#endif // __STEAMSTUB32VARIANT3_H_INCLUDED__