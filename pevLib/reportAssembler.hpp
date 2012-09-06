//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once
#include "reportComponents.hpp"

namespace vFind
{

class QueryChain
{
	std::vector<IInputProvider*> inputs;
	std::vector<IFilter* > filters;
	std::vector<IOutput* > outputs;

	void RunInput(IInputProvider&);

public:
	~QueryChain();

	void Install(std::auto_ptr<IInputProvider> newInput);
	void Install(std::auto_ptr<IFilter> newFilter);
	void Install(std::auto_ptr<IOutput> newOutput);

	void Run();
};

}
