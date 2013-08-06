//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// pevFind.cpp -- Defines the main entry point for the executable. Bootstraps
// associated subprograms.

#include <iostream>
#include <fstream>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "../pevLib/utility.h"

//Subprogram definitions
#include "../pevLib/clsidCompressor.h"
#include "../pevLib/procListers.h"
#include "../pevLib/exec.h"
#include "../pevLib/vFind.h"
#include "../pevLib/link.h"
#include "../pevLib/volumeEnumerate.h"
#include "../pevLib/uZip.h"
#include "../pevLib/times.h"
#include "../pevLib/regImport.h"
#include "../pevLib/rexport.h"
#include "../pevLib/moveex.h"
#include "../pevLib/serviceControl.h"
#include "../pevLib/dosdev.h"
#include "../pevLib/linkResolve.h"
#include "../pevLib/wait.hpp"

int __cdecl wmain(int argc, wchar_t* argv[])
{
    using namespace boost::algorithm;
    try {
    if (argc < 2) throw std::runtime_error("Invalid number of arguments."); //We always require at least one option.
    
    if (iequals(argv[1], L"vfind")) //vFind is the default, and therefore we don't need to bother reallocating
        return vFind::main(); //It's argument pointers and such.
    
    boost::iterator_range<wchar_t *> firstArgument(argv[1], argv[1] + wcslen(argv[1]));
    argc -= 1;
    argv += 1;
    
    if      (iequals(firstArgument, L"CLSID"))
        return clsidCompressor::main(argc, argv);
    else if (iequals(firstArgument, L"EXEC"))
        return exec::main();
    else if (iequals(firstArgument, L"PLIST"))
        return plist::main(argc, argv);
    else if (iequals(firstArgument, L"CLIST"))
        return clist::main(argc, argv);
    else if (iequals(firstArgument, L"LINK"))
        return link::main(argc, argv);
    else if (iequals(firstArgument, L"VOLUME"))
        return volumeEnumerate::main();
    else if (iequals(firstArgument, L"UZIP"))
        return uZip::main(argc, argv);
    else if (iequals(firstArgument, L"TIME"))
        return times::main(argc, argv);
    else if (iequals(firstArgument, L"RIMPORT"))
        return regImport::main(argc, argv, KEY_WOW64_32KEY);
    else if (iequals(firstArgument, L"RIMPORT32"))
        return regImport::main(argc, argv, KEY_WOW64_32KEY);
    else if (iequals(firstArgument, L"RIMPORT64"))
        return regImport::main(argc, argv, KEY_WOW64_64KEY);
    else if (iequals(firstArgument, L"MOVEEX"))
        return moveex::main(argc, argv);
    else if (iequals(firstArgument, L"SC"))
        return serviceControl::main(argc, argv);
    else if (iequals(firstArgument, L"DDEV"))
        return dosdev::main(argc, argv);
    else if (iequals(firstArgument, L"LINKRESOLVE"))
        return linkResolve::main(argc, argv);
    else if (iequals(firstArgument, L"REXPORT"))
        return rexport::main(argc, argv);
    else if (iequals(firstArgument, L"WAIT"))
        return wait::main(argc, argv);
    return vFind::main();
    }
    catch (std::exception& except)
    {
#define STRINGIZE_LITERAL(x) #x
#define STRINGIZE(x) STRINGIZE_LITERAL(x)
        std::cerr << 
        "                         _____ _           _ \n"
        "        _ __   _____   _|  ___(_)_ __   __| |\n"
        "       | '_ \\ / _ \\ \\ / / |_  | | '_ \\ / _` |\n"
        "       | |_) |  __/\\ V /|  _| | | | | | (_| |\n"
        "       | .__/ \\___| \\_/ |_|   |_|_| |_|\\__,_|\n"
        "       |_|    by Billy Robert O'Neal III\n"
        "                      Version " STRINGIZE(PEVFIND_VERSION) "\n"
        "  Distributed under the Boost Software License, Version 1.0.\n"
        "         http://www.boost.org/LICENSE_1_0.txt\n"
        "pevFind contains some code from Info-ZIP, used with permission.\n"
        "  In accordance with Info-ZIP's License, it can be found at\n"
        "           http://billy-oneal.com/infozip.txt\n"
        "            Filename regular expressions library is\n"
        " Copyright (C)1997-1998 by David R. Tribble, all rights reserved.\n\n";
        std::cerr << except.what() << "\n";
    }
}
