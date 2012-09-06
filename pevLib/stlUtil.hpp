//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once
#include <functional>

template <typename T>
struct Deleter : public std::unary_function<void, T>
{
	void operator()(T t)
	{
		delete t;
	}
};
