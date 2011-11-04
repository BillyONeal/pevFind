//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "pch.hpp"
#include "fileData.h"
#include "reportComponents.hpp"

namespace vFind {

std::vector<FileData> IFilter::Results()
{
	return std::vector<FileData>();
}


FileInput::FileInput( const std::wstring& rootPath )
{
	hSearch = FindFirstFile(rootPath.c_str(), &data);
}

}
