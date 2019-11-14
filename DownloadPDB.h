#pragma once
#include <Windows.h>
#include <tchar.h>

EXTERN_C char* GetPdbName(const wchar_t* filename, char* pdburl, int buflen);