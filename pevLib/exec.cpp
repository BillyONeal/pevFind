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
            state = 2;
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
    std::wstring::const_iterator process(std::wstring::const_iterator begin, std::wstring::const_iterator end);
};

static int execute(const std::wstring& commandLine, const commandlineProcessor& options);

void initProcessor(std::wstring& commandLine, commandlineProcessor& options)
{
    commandLine.assign(GetCommandLine());
    std::wstring::const_iterator endOfOptions = commandLine.begin();
    //pevFind.exe
    endOfOptions = findEndOfCommandline(endOfOptions, commandLine.end());
    //EXEC
    endOfOptions = findEndOfCommandline(endOfOptions, commandLine.end());
    
    options.e = false; //Clear
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
    wchar_t * environment;
    HANDLE userToken;

    if (options.s)
    {
        // Impersonate SYSTEM by stealing the token from csrss.
    }

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
    std::vector<wchar_t> commandLineBuffer(commandLine.cbegin(), commandLine.cend());
    commandLineBuffer.push_back(L'\0');
    noError = ::CreateProcessW(
        nullptr,                                       //lpApplicationName
        commandLineBuffer.data(),                      //lpCommandLine
        nullptr,                                       //lpProcessAttributes
        nullptr,                                       //lpThreadAttributes
        FALSE,                                         //bInheritHandles
        CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT, //dwCreationFlags
        options.e ? environment : NULL,                //lpEnvironment
        nullptr,                                       //lpCurrentDirectory
        &startupInfo,                                  //lpStartupInfo
        &processInformation                            //lpProcessInformation
    );

    if(noError == 0)
    {
        return GetLastError();
    }
    
    DWORD exitCode = 0;
    
    if (options.w)
    {
        WaitForSingleObject(processInformation.hProcess, INFINITE);
        if (GetExitCodeProcess(processInformation.hProcess, &exitCode) == 0)
        {
            exitCode = static_cast<DWORD>(-1);
        }
    }

    CloseHandle( processInformation.hProcess );
    CloseHandle( processInformation.hThread );
    if (options.e)
    {
        DestroyEnvironmentBlock(static_cast<LPVOID>(environment));
    }

    return static_cast<int>(exitCode);
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

} //namespace exec
