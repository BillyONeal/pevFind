//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// Provides classes for managing processes

#include "pch.hpp"
#include <string>
#include <vector>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "processManager.h"

_NtOpenProcess NtOpenProcess;
_NtQuerySystemInformation NtQuerySystemInformation;
_NtClose NtClose;
_NtOpenProcessToken NtOpenProcessToken;
_NtAdjustPrivilegesToken NtAdjustPrivilegesToken;
_NtQueryInformationProcess NtQueryInformationProcess;
_NtTerminateProcess NtTerminateProcess;

process::process(const SYSTEM_PROCESS_INFORMATION& procInfo)
{
    processID = (DWORD) procInfo.ProcessId;
};

std::wstring process::commandLine() const
{
    NTSTATUS errorCheck;
    setupHandle(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_OPERATION);
    PROCESS_BASIC_INFORMATION procInfo;
    errorCheck = NtQueryInformationProcess(hProc,ProcessBasicInformation,&procInfo,sizeof(procInfo),NULL);
    if (errorCheck)
        throw std::runtime_error("Failed to query process info.");
    PEB envBlock;
    if (!ReadProcessMemory(hProc,procInfo.PebBaseAddress,(LPVOID)&envBlock,sizeof(PEB),NULL))
        throw std::runtime_error("Failed to copy the Process Environment Block!");
    RTL_USER_PROCESS_PARAMETERS params;
    if (!ReadProcessMemory(hProc,(LPVOID)envBlock.ProcessParameters,(LPVOID)&params,sizeof(params),NULL))
        throw std::runtime_error("Failed to copy the RTL_USER_PROCESS_PARAMETERS!");
    wchar_t strInner[32767L];
    if (!ReadProcessMemory(hProc,(LPVOID)params.CommandLine.Buffer,(LPVOID)strInner,params.CommandLine.Length,NULL))
        throw std::runtime_error("Failed to copy the Commandline!");
    teardownHandle();
    return std::wstring(strInner,params.CommandLine.Length/sizeof(TCHAR));
};

std::wstring process::executablePath() const
{
    NTSTATUS errorCheck;
    setupHandle(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_OPERATION);
    PROCESS_BASIC_INFORMATION procInfo;
    errorCheck = NtQueryInformationProcess(hProc,ProcessBasicInformation,&procInfo,sizeof(procInfo),NULL);
    if (errorCheck)
        throw std::runtime_error("Failed to query process info.");
    PEB envBlock;
    if (!ReadProcessMemory(hProc,procInfo.PebBaseAddress,(LPVOID)&envBlock,sizeof(PEB),NULL))
        throw std::runtime_error("Failed to copy the Process Environment Block!");
    RTL_USER_PROCESS_PARAMETERS params;
    if (!ReadProcessMemory(hProc,(LPVOID)envBlock.ProcessParameters,(LPVOID)&params,sizeof(params),NULL))
        throw std::runtime_error("Failed to copy the RTL_USER_PROCESS_PARAMETERS!");
    wchar_t strInner[32767L];
    if (!ReadProcessMemory(hProc,(LPVOID)params.ImagePathName.Buffer,(LPVOID)strInner,params.ImagePathName.Length,NULL))
        throw std::runtime_error("Failed to copy the ImagePath!");
    teardownHandle();
    return std::wstring(strInner,params.ImagePathName.Length/sizeof(TCHAR));
}

processManager::processManager()
{
    //Load NTDLL.DLL
    hNTDLL = LoadLibrary(L"ntdll.dll");
    if (!hNTDLL)
        throw std::runtime_error("NTDLL.DLL failure.");

    //Load NTOpenProcess
    NtOpenProcess = reinterpret_cast <_NtOpenProcess> (GetProcAddress(hNTDLL, "NtOpenProcess")); 
    if (!NtOpenProcess)
        throw std::runtime_error("NtOpenProcess not found.");

    //Load NtQuerySystemInformation
    NtQuerySystemInformation = reinterpret_cast <_NtQuerySystemInformation> (GetProcAddress(hNTDLL, "NtQuerySystemInformation"));
    if (!NtQuerySystemInformation)
        throw std::runtime_error("NtQuerySystemInformation not found.");

    //Load NtOpenProcessToken
    NtOpenProcessToken = reinterpret_cast<_NtOpenProcessToken>(GetProcAddress(hNTDLL, "NtOpenProcessToken"));
    if (!NtOpenProcessToken)
        throw std::runtime_error("NtOpenProcessToken not found.");

    //Load NtAdjustPrivilegesToken
    NtAdjustPrivilegesToken = reinterpret_cast<_NtAdjustPrivilegesToken>(GetProcAddress(hNTDLL, "NtAdjustPrivilegesToken"));
    if (!NtAdjustPrivilegesToken)
        throw std::runtime_error("NtAdjustPrivilegesToken not found.");

    //Load NtQueryInformationProcess

    NtQueryInformationProcess = reinterpret_cast<_NtQueryInformationProcess>(GetProcAddress(hNTDLL, "NtQueryInformationProcess"));
    if (!NtQueryInformationProcess)
        throw std::runtime_error("NtQueryInformationProcess not found.");

    //Load NtTerminateProcess
    NtTerminateProcess = reinterpret_cast<_NtTerminateProcess>(GetProcAddress(hNTDLL, "NtTerminateProcess"));
    if (!NtTerminateProcess)
        throw std::runtime_error("NtTerminateProcess not found.");

    //Load NtClose
    NtClose = reinterpret_cast<_NtClose>(GetProcAddress(hNTDLL, "NtClose"));
    if (!NtClose)
        throw std::runtime_error("NtClose not found.");

    //Grant Myself SeDebugPrivs

    CLIENT_ID clID;
    OBJECT_ATTRIBUTES objAtt;
    HANDLE curProc;
    HANDLE curProcToken;
    TOKEN_PRIVILEGES privies;

    ZeroMemory(&clID, sizeof(CLIENT_ID));
    ZeroMemory(&objAtt, sizeof(OBJECT_ATTRIBUTES));
    objAtt.Length = sizeof(OBJECT_ATTRIBUTES);
    ZeroMemory(&privies, sizeof(privies));

    //Open myself
    clID.UniqueProcess = (HANDLE) GetCurrentProcessId();
    NTSTATUS errorCheck = NtOpenProcess(&curProc, PROCESS_SET_INFORMATION | PROCESS_QUERY_INFORMATION, &objAtt, &clID);
    if (errorCheck)
        throw std::runtime_error("Could not open my own process to grant SeDebugPrivs");
    //Open my token
    errorCheck = NtOpenProcessToken(curProc, TOKEN_ADJUST_PRIVILEGES, &curProcToken);
    if (errorCheck)
        throw std::runtime_error("Could not open my own process to grant SeDebugPrivs");

    //Initialize a new structure with SeDebug stuff
    privies.PrivilegeCount = 1;
    LookupPrivilegeValue(NULL,L"SeDebugPrivilege",&privies.Privileges->Luid);
    privies.Privileges->Attributes = SE_PRIVILEGE_ENABLED;

    //Adjust and close
    errorCheck = NtAdjustPrivilegesToken(curProcToken,NULL,&privies,NULL,NULL,NULL);
    if (errorCheck)
        throw std::runtime_error("Could not grant myself SeDebugPrivs");
    NtClose(curProcToken);
    NtClose(curProc);
};

std::vector<process> processManager::enumerate() const
{
    ULONG sizeAlloc;
    sizeAlloc = 20480;
    PVOID procs = NULL;
    LONG errorCheck = 0;
    for(;;)
    {
        procs = new BYTE[sizeAlloc];
        errorCheck = NtQuerySystemInformation(SystemProcessInformation, procs, sizeAlloc, NULL);
        if (errorCheck == 0)
            break;
        else if (errorCheck != 0xC0000004)
            throw std::runtime_error("Programmer toast. Terminate Bill.");
        delete [] procs;
        sizeAlloc += 4096;
    }
    LPBYTE procCursor = (LPBYTE)procs;
    std::vector<process> results;
    if (((PSYSTEM_PROCESS_INFORMATION)procCursor)->ProcessId)
        results.push_back(*((PSYSTEM_PROCESS_INFORMATION)procCursor));
    while (((PSYSTEM_PROCESS_INFORMATION)procCursor)->NextEntryOffset)
    {
        procCursor += ((PSYSTEM_PROCESS_INFORMATION)procCursor)->NextEntryOffset;
        if (((PSYSTEM_PROCESS_INFORMATION)procCursor)->ProcessId)
            results.push_back(*((PSYSTEM_PROCESS_INFORMATION)procCursor));
    }
    delete [] procs;
    return results;
}

DWORD WINAPI killActor(__in  LPVOID lpParameter)
{
    DWORD idToKill = (DWORD) lpParameter;
    HANDLE hProc;
    OBJECT_ATTRIBUTES objAttr;
    NTSTATUS errorCheck;
    ZeroMemory(&objAttr, sizeof(OBJECT_ATTRIBUTES));
    objAttr.Length = sizeof(OBJECT_ATTRIBUTES);
    CLIENT_ID clID;
    ZeroMemory(&clID, sizeof(CLIENT_ID));
    clID.UniqueProcess = (HANDLE)idToKill;
    errorCheck = NtOpenProcess(&hProc,
        PROCESS_TERMINATE,
        &objAttr,
        &clID);
    if (errorCheck)
        throw std::runtime_error("Failed to open process.");
    DWORD result = NtTerminateProcess(hProc, 0);
    NtClose(hProc);
    return result;
}

bool process::kill()
{
    bool result = true;
    HANDLE hThread = CreateThread(NULL,0,&killActor,(LPVOID)processID,NULL, NULL);
    if (WaitForSingleObject(hThread, 10000) == WAIT_TIMEOUT) //Give it 10 secs to finish
    {
        result = false;
        TerminateThread(hThread, 0);
    }
    CloseHandle(hThread);
    return result;
}

HANDLE process::setupHandle(ACCESS_MASK mask) const
{
    OBJECT_ATTRIBUTES objAttr;
    NTSTATUS errorCheck;
    ZeroMemory(&objAttr, sizeof(OBJECT_ATTRIBUTES));
    objAttr.Length = sizeof(OBJECT_ATTRIBUTES);
    CLIENT_ID clID;
    ZeroMemory(&clID, sizeof(CLIENT_ID));
    clID.UniqueProcess = (HANDLE) processID;
    errorCheck = NtOpenProcess(&hProc,
        mask,
        &objAttr,
        &clID);
    if (errorCheck)
        throw std::runtime_error("Failed to open process.");
    return hProc;
}

void process::teardownHandle() const
{
    NtClose(hProc);
}

