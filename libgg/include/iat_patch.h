#pragma once

#include <Windows.h>


void* PatchIAT(HMODULE module, void* oldSymbol, void* newSymbol);

template<class F>
F* PatchIAT(LPCSTR symbolModule, LPCSTR symbolName, LPCSTR iatModuleName, F* replacement)
{
    void* import = ::GetProcAddress(::GetModuleHandleA(symbolModule), symbolName);
    if (import == NULL)
        return NULL;

    return (F*)PatchIAT(::GetModuleHandleA(iatModuleName), import, replacement);
}
