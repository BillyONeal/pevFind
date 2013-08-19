//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// Provides plist and clist subprograms

#include "pch.hpp"
#include <vector>
#include <cstdio>
#include "../LogCommon/Win32Exception.hpp"
#include "../LogCommon/Process.hpp"

namespace
{

    typedef std::wstring(Instalog::SystemFacades::Process::* ProcessFunction)() const;

    static int DoProcessList(int argc, wchar_t * argv[], ProcessFunction member)
    {
        FILE* output;
        if (argc > 1)
        {
            _wfopen_s(&output, argv[1], L"w");
        }
        else
        {
            output = stdout;
        }
        Instalog::SystemFacades::ProcessEnumerator enumerator;
        for (auto const& process : enumerator)
        {
            try
            {
                std::wstring str((process.*member)());
                clearerr_s(output);
                fwprintf_s(output, L"%s\n", str.c_str());
                fflush(output);
            }
            catch (Instalog::SystemFacades::Win32Exception const&)
            {
            }
        }

        return 0;
    }

}

namespace plist {

    int main(int argc, wchar_t * argv[])
    {
        return DoProcessList(argc, argv, &Instalog::SystemFacades::Process::GetExecutablePath);
    }

};

namespace clist {

    int main(int argc, wchar_t * argv[])
    {
        return DoProcessList(argc, argv, &Instalog::SystemFacades::Process::GetCmdLine);
    }

};
