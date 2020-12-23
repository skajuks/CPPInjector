#pragma once

#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>

typedef HMODULE(__stdcall* pLoadLibraryA)(LPCSTR);
typedef FARPROC(__stdcall* pGetProcAddress)(HMODULE, LPCSTR);

typedef INT(__stdcall* dllmain)(HMODULE, DWORD, LPVOID);


struct loaderdata
{
    LPVOID ImageBase;

    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_BASE_RELOCATION BaseReloc;
    PIMAGE_IMPORT_DESCRIPTOR ImportDirectory;

    pLoadLibraryA fnLoadLibraryA;
    pGetProcAddress fnGetProcAddress;

};

DWORD __stdcall LibraryLoader(LPVOID Memory);
DWORD __stdcall stub();
