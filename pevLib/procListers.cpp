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
#include "procListers.h"
#include "processManager.h"

namespace plist {

    int main(int argc, wchar_t * argv[])
    {
        std::wostream *output;
        std::auto_ptr<std::wostream> destroyer;
        if (argc > 1)
        {
            destroyer.reset(new std::wofstream(argv[1]));
            output = destroyer.get();
        }
        else
        {
            output = &std::wcout;
        }
        processManager manager;
        std::vector<process> processes(manager.enumerate());
        for (std::vector<process>::const_iterator it = processes.begin(); it != processes.end(); it++)
        {
            try
            {
                *output << it->executablePath() << std::endl;
                output->clear();
            }
            catch (std::runtime_error pain)
            {
            }
        }
        return 0;
    }

};

namespace clist {

    int main()
    {
        processManager manager;
        std::vector<process> procs(manager.enumerate());
        for (std::vector<process>::const_iterator it = procs.begin(); it != procs.end(); it++)
        {
            try
            {
                std::wcout << it->commandLine() << std::endl;
                std::wcout.clear();
            }
            catch (std::runtime_error pain)
            {
            }
        }
        return 0;
    }

};
