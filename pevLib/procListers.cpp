//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// Provides plist and clist subprograms

#include "pch.hpp"
#include <vector>
#include "../LogCommon/Win32Exception.hpp"
#include "../LogCommon/Process.hpp"

namespace plist {

    int main(int argc, wchar_t * argv[])
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
                std::fwprintf(output, L"%s\n", process.GetExecutablePath().c_str());
            }
            catch (Instalog::SystemFacades::Win32Exception const&)
            {
            }
        }
        return 0;
    }

};

namespace clist {

    int main(int argc, wchar_t * argv[])
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
                std::fwprintf(output, L"%s\n", process.GetCmdLine().c_str());
            }
            catch (Instalog::SystemFacades::Win32Exception const&)
            {
            }
        }
        return 0;
    }

};
