#include "DownloadPDB.h"
#include <stdio.h>
#include <conio.h>
#include "defs.h"
#include <Urlmon.h>
#include <Shlwapi.h>
#pragma comment(lib, "Urlmon.lib")
#pragma comment(lib, "shlwapi")

BOOL DownFile(const wchar_t* dllName)
{
    BOOL retval = FALSE;
    CHAR* pchPdbUrl = (char*)AllocMemory(4096);
    LPWSTR lpPdbName = (LPWSTR)AllocMemory((wcslen(dllName)+MAX_PATH)*sizeof(WCHAR));
    if (pchPdbUrl && lpPdbName)
    {
        wcscpy(lpPdbName, dllName);
        if (GetPdbName(dllName, pchPdbUrl, 4096))
        {
            printf("%s\n",  pchPdbUrl);

            // обть 
            LPWSTR lpExt = PathFindExtensionW(lpPdbName);
            if (lpExt)
            {
                wcscpy(lpExt, L".pdb");
            }
            else
            {
                wcscat(lpPdbName, L".pdb");
            }
            WCHAR* wzpchPdbUrl = (WCHAR*)AllocMemory(4096*sizeof(WCHAR));
            if (wzpchPdbUrl)
            {
                MultiByteToWideChar( CP_ACP,
                    0,
                    pchPdbUrl,
                    -1,
                    wzpchPdbUrl,
                    4096 );
                if ( S_OK == URLDownloadToFileW(NULL, wzpchPdbUrl, lpPdbName, 0, 0) )
                {
                    retval = TRUE;
                }
                FreeMemory(wzpchPdbUrl);
            }
        }
    }
    FreeMemory(pchPdbUrl);
    FreeMemory(lpPdbName);
    return retval;
}


DWORD EnumFile(LPCWSTR lppath)
{
    DWORD nCount = 0;
    DWORD dwLen = wcslen(lppath);
    LPWSTR lpFindName = (LPWSTR)AllocMemory(sizeof(WCHAR)*(dwLen+MAX_PATH+4));
    if (lpFindName == NULL)
    {
        return 0;
    }

    wcscpy(lpFindName, lppath);
    while (dwLen>0 && ( lpFindName[dwLen-1] ==L'\\' || lpFindName[dwLen-1] == L'/'))
    {
        lpFindName[--dwLen] = L'\0';
    }
    lpFindName[dwLen++] = L'\\';
    wcscpy(lpFindName+dwLen, L"*.*");

    WIN32_FIND_DATAW FindFileData = {0};
    HANDLE hFind = FindFirstFileW(lpFindName, &FindFileData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do 
        {
            if (
                wcscmp(FindFileData.cFileName, L".") == 0 || 
                wcscmp(FindFileData.cFileName, L"..") == 0
                )
            {
                continue;
            }

            wcscpy(lpFindName+dwLen, FindFileData.cFileName);
            if ( FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
            {
                EnumFile(lpFindName);
            }
            else
            {
                DownFile(lpFindName);
            }
        } while (FindNextFileW(hFind, &FindFileData));
    }
    return nCount;
}

int wmain( int argc, wchar_t *argv[ ], wchar_t *envp[ ] )
{
    if (argc<2)
    {
        printf("usage : dPDB <FileName>\n");
        return 0;
    }

    LPWSTR lpPath = (LPWSTR)AllocMemory(MAX_PATH*2*sizeof(WCHAR));
    if (!lpPath)
    {
        printf("memory alloc error!\n");
        return 0;
    }

    GetModuleFileNameW(GetModuleHandle(NULL), lpPath, MAX_PATH);
    PathRemoveFileSpecW(lpPath);
    wcscat(lpPath, L"\\");
    size_t curlen = wcslen(lpPath);

    for (int i=1; i<argc; i++)
    {
        LPCWSTR lpFileStr = argv[i];
        if (lpFileStr[1] != L':')
        {
            wcscpy(lpPath+curlen, lpFileStr);
            lpFileStr = lpPath;
        }
        if (PathIsDirectoryW(lpFileStr))
        {
            EnumFile(lpFileStr);
        }
        else
        {
            DownFile(lpFileStr);
        }
    }
    FreeMemory(lpPath);
    return 0;
}