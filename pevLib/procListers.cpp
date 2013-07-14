//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// Provides plist and clist subprograms

#include "pch.hpp"
#include <vector>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include "../LogCommon/Win32Exception.hpp"
#include "../LogCommon/Process.hpp"

namespace plist {

    int main(int argc, wchar_t * argv[])
    {
        std::wostream *output;
        std::unique_ptr<std::wostream> destroyer;
        if (argc > 1)
        {
            destroyer.reset(new std::wofstream(argv[1]));
            output = destroyer.get();
        }
        else
        {
            output = &std::wcout;
        }
		Instalog::SystemFacades::ProcessEnumerator enumerator;
		for (auto& process : enumerator)
        {
            try
            {
                *output << process.GetExecutablePath() << std::endl;
                output->clear();
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
		std::wostream *output;
		std::unique_ptr<std::wostream> destroyer;
		if (argc > 1)
		{
			destroyer.reset(new std::wofstream(argv[1]));
			output = destroyer.get();
		}
		else
		{
			output = &std::wcout;
		}
		Instalog::SystemFacades::ProcessEnumerator enumerator;
		for (auto& process : enumerator)
		{
			try
			{
				*output << process.GetCmdLine() << std::endl;
				output->clear();
			}
			catch (Instalog::SystemFacades::Win32Exception const&)
			{
			}
		}
		return 0;
    }

};
