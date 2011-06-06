#ifndef _LINK_RESOLVE_H_INCLUDED
#define _LINK_RESOLVE_H_INCLUDED
//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// linkResolve.h -- Defines the linkResolve subprogram and library function.

#include <string>

namespace linkResolve
{
	int main(int argc, wchar_t * argv[]);
	std::wstring resolveLink(std::wstring& lnkPath);
};

#endif //_LINK_RESOLVE_H_INCLUDED
