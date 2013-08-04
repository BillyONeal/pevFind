//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// exec.cpp -- Implements the exec subprogram

#include "pch.hpp"
#include <stdexcept>
#include <string>
#include <windows.h>
#include <Userenv.h>

#include "exec.h"

static const wchar_t serviceName[] = L"PEVSystemStart";

VOID WINAPI SvcHandler(DWORD fdwControl);
VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR *lpszArgv);

namespace exec {

std::wstring::const_iterator findEndOfCommandline(std::wstring::const_iterator it, std::wstring::const_iterator end)
{
    unsigned int state = 0;
    wchar_t currentCharacter;
    for (; it != end; it++)
    {
        /*
         * This is a simple state machine which decides where the end of the commandline is.
         * For more info on processing commandlines, see
         * http://msdn.microsoft.com/en-us/library/17w5ykft.aspx
         */
        currentCharacter = *it;
        switch(state)
        {
        case 0:
            switch(currentCharacter)
            {
            case L'"':
                state = 1;
                continue;
            case L' ':
            case L'\t':
            case L'\n':
            case L'\r':
                continue;
            default:
                state = 2;
                continue;
            }
            break;
        case 1:
            switch(currentCharacter)
            {
            case L'"':
                state = 3;
                continue;
            case L'\\':
                state = 4;
                continue;
            default:
                continue;
            }
            break;
        case 2:
            switch(currentCharacter)
            {
            case L' ':
            case L'\t':
            case L'\n':
            case L'\r':
                return it;
            default:
                continue;
            }
            break;
        case 3:
            switch(currentCharacter)
            {
            case L'"':
                state = 1;
                continue;
            default:
                return it;
            }
            break;
        case 4:
            state = 1;
            break;
        default:
            throw std::logic_error("Bill's got a bad bug!");
        }
    }
    return it;
}

class commandlineProcessor
{
public:
    bool s; // Start the process as System
    bool e; // Start the process using a new environment
    bool w; // Wait for the process after starting it
    bool i; // Internal
    std::wstring::const_iterator process(std::wstring::const_iterator begin, std::wstring::const_iterator end);
};

int systemExecute(const std::wstring& commandLine);
int serviceExecute(const std::wstring& commandLine);
int execute(const std::wstring& commandLine, const commandlineProcessor& options);

void initProcessor(std::wstring& commandLine, commandlineProcessor& options)
{
    commandLine.assign(GetCommandLine());
    std::wstring::const_iterator endOfOptions = commandLine.begin();
    //pevFind.exe
    endOfOptions = findEndOfCommandline(endOfOptions, commandLine.end());
    //EXEC
    endOfOptions = findEndOfCommandline(endOfOptions, commandLine.end());
    
    options.e = false; //Clear
    options.i = false;
    options.s = false;
    options.w = false;
    endOfOptions = options.process(endOfOptions, commandLine.end());
    
    //Sanity check
    if (endOfOptions == commandLine.end())
    {
        throw std::invalid_argument("Some form of argument must be specified.");
    }	
    
    commandLine.erase(commandLine.begin(), endOfOptions);
}

int main()
{
    std::wstring commandLine;
    commandlineProcessor options;
    initProcessor(commandLine, options);
    if (options.s)
    {
        return systemExecute(commandLine);
    }
    else if (options.i)
    {
        return serviceExecute(commandLine);
    }
    else
    {
        return execute(commandLine, options);
    }
}

std::wstring::const_iterator commandlineProcessor::process(std::wstring::const_iterator begin, std::wstring::const_iterator end)
{
    //Sanity check
    if (begin == end) return end;
    unsigned __int32 state = 0;
    for(; begin != end; begin++)
    {
        switch(state)
        {
        case 0:
            switch(*begin)
            {
            case L'/':
            case L'-':
                state = 1;
                break;
            case L'\n':
            case L'\t':
            case L' ':
                break;
            default: return begin;
            }
            break;
        case 1:
            switch(*begin)
            {
            case L'S':
            case L's':
                s = true;
                break;
            case L'E':
            case L'e':
                e = true;
                break;
            case L'W':
            case L'w':
                w = true;
                break;
            case L'I':
            case L'i':
                i = true;
            case L'\n':
            case L'\t':
            case L' ':
                state = 0;
                break;
            default: return begin;
            }
            break;
        }
    }
    return end;
}

int systemExecute(const std::wstring& commandLine)
{	
    //Calculate the commandline used for the service.
    wchar_t moduleName[MAX_PATH];
    if (!GetModuleFileName(NULL, moduleName, MAX_PATH)) return -1;
    std::wstring serviceString(L"\"");
    serviceString.append(moduleName).append(L"\" EXEC /i ").append(commandLine);
    
    //Open the SCM and create the service
    SC_HANDLE hScm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE | STANDARD_RIGHTS_EXECUTE);
    
    //Check if a service by that name already exists and delete it if so
    SC_HANDLE hOldSvc = OpenService(hScm,serviceName, SERVICE_STOP | DELETE);
    if (hOldSvc)
    {
        SERVICE_STATUS garbage;
        ControlService(hOldSvc, SERVICE_CONTROL_STOP, &garbage);
        DeleteService(hOldSvc);
        CloseServiceHandle(hOldSvc);
    }
    
    SC_HANDLE hSvc = CreateService(hScm, serviceName, NULL, DELETE| SERVICE_START | SERVICE_QUERY_STATUS,
    SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_IGNORE,
    serviceString.c_str(), NULL, NULL, NULL, L"LocalSystem", NULL);
    if (!hSvc)
    {
        return GetLastError();
    }
    
    //Create safe mode bindings for it
    HKEY safebootKey;
    RegCreateKeyEx(HKEY_LOCAL_MACHINE,L"SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal\\PEVSystemStart", 0, NULL, REG_OPTION_VOLATILE, KEY_SET_VALUE, NULL, &safebootKey, NULL);
    RegSetValueEx(safebootKey, NULL, 0, REG_SZ, reinterpret_cast<const BYTE *>(L"Service"), 8 * sizeof(wchar_t));
    RegCloseKey(safebootKey);
    RegCreateKeyEx(HKEY_LOCAL_MACHINE,L"SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network\\PEVSystemStart", 0, NULL, REG_OPTION_VOLATILE, KEY_SET_VALUE, NULL, &safebootKey, NULL);
    RegSetValueEx(safebootKey, NULL, 0, REG_SZ, reinterpret_cast<const BYTE *>(L"Service"), 8 * sizeof(wchar_t));
    RegCloseKey(safebootKey);
    
    int exitCode;
    exitCode = (int) StartService(hSvc, 0, NULL);
    
    SERVICE_STATUS sStatus;
    do
    {
        Sleep(100);
        QueryServiceStatus(hSvc, &sStatus);
    }
    while (sStatus.dwCurrentState != SERVICE_STOPPED);
    DeleteService(hSvc);
    CloseServiceHandle(hSvc);
    CloseServiceHandle(hScm);
    return 0;
}

int execute(const std::wstring& commandLine, const commandlineProcessor& options)
{
    wchar_t * environment = nullptr;
    HANDLE userToken = INVALID_HANDLE_VALUE;

    if (options.e)
    {
        OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &userToken);
        CreateEnvironmentBlock(
            reinterpret_cast<LPVOID *>(&environment),
            userToken,
            FALSE
        );
        CloseHandle(userToken);
    }


    BOOL noError;
    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInformation;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESHOWWINDOW;
    startupInfo.wShowWindow = SW_HIDE;
    noError = CreateProcess(
        NULL,											//lpApplicationName
        const_cast<LPWSTR>(commandLine.c_str()),		//lpCommandLine
        NULL,											//lpProcessAttributes
        NULL,											//lpThreadAttributes
        FALSE,											//bInheritHandles
        CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT,	//dwCreationFlags
        options.e ? environment : NULL,					//lpEnvironment
        NULL,											//lpCurrentDirectory
        &startupInfo,									//lpStartupInfo
        &processInformation								//lpProcessInformation
    );

    if(!noError)
    {
        return GetLastError();
    }
    
    DWORD exitCode = 0;
    
    if (options.w)
    {
        WaitForSingleObject(processInformation.hProcess, INFINITE);
        if (GetExitCodeProcess(processInformation.hProcess, &exitCode) == 0)
        {
            exitCode = (DWORD)-1;
        }
    }

    CloseHandle( processInformation.hProcess );
    CloseHandle( processInformation.hThread );
    if (options.e)
    {
        DestroyEnvironmentBlock(static_cast<LPVOID>(environment));
    }

    return (int) exitCode;
}

int serviceExecute(const std::wstring&)
{
    SERVICE_TABLE_ENTRY DispatchTable[] = 
    { 
        { L"PEVSystemStart", (LPSERVICE_MAIN_FUNCTION) SvcMain }, 
        { NULL, NULL } 
    }; 
    if (!StartServiceCtrlDispatcher( DispatchTable )) 
    { 
        return -1;
    }
    return 0;
}

} //namespace exec

VOID WINAPI SvcMain(DWORD, LPTSTR *)
{
    using namespace exec;
    SERVICE_STATUS_HANDLE hStatus;
    SERVICE_STATUS sStatus;
    
    //Set initial values
    sStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS;
    sStatus.dwCurrentState = SERVICE_START_PENDING;
    sStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    sStatus.dwWin32ExitCode = 0;
    sStatus.dwServiceSpecificExitCode = 0;
    sStatus.dwCheckPoint = 0;
    sStatus.dwWaitHint = 0;

    //Register the service
    hStatus = RegisterServiceCtrlHandler(serviceName, SvcHandler);
    if (!hStatus) return;
    
    //Status messages to make the SCM happy.
    SetServiceStatus(hStatus, &sStatus);
    sStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(hStatus, &sStatus);

    std::wstring commandLine;
    commandlineProcessor options;

    initProcessor(commandLine, options);

    execute(commandLine, options);

    //Report that we're done.
    sStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(hStatus, &sStatus);
}


VOID WINAPI SvcHandler(DWORD)
{
    return;
}
