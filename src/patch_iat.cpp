#include "patch_iat.h"

#include <Dbghelp.h>


void* PatchIAT(HMODULE module, void* oldSymbol, void* newSymbol)
{
    ULONG size = 0;
    unsigned char* baseAddress = (unsigned char*)module;
    void* originalSymbol = NULL;

    IMAGE_IMPORT_DESCRIPTOR* importDescriptor = (IMAGE_IMPORT_DESCRIPTOR*)::ImageDirectoryEntryToData(
        baseAddress, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &size);
    if (importDescriptor == NULL)
    {
        //Patching module has no IDT.
        return NULL;
    }

    while (importDescriptor->FirstThunk != 0)
    {
        IMAGE_THUNK_DATA* thunk = (IMAGE_THUNK_DATA*)(baseAddress + importDescriptor->FirstThunk);
        while (thunk->u1.Function != 0)
        {
            if (thunk->u1.Function != (DWORD_PTR)oldSymbol) 
            {
                thunk++;
                continue;
            }

            originalSymbol = (LPVOID)thunk->u1.Function;
            DWORD protect;
            ::VirtualProtect(&thunk->u1.Function, sizeof(thunk->u1.Function), PAGE_EXECUTE_READWRITE, &protect);
            thunk->u1.Function = (DWORD_PTR)newSymbol;
            ::VirtualProtect(&thunk->u1.Function, sizeof(thunk->u1.Function), protect, &protect);
        }
        importDescriptor++;
    }

    return originalSymbol;
}