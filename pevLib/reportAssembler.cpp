//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "pch.hpp"
#include "stlUtil.hpp"
#include "reportAssembler.hpp"

namespace vFind {

void QueryChain::Install( std::auto_ptr<IInputProvider> newInput )
{
    inputs.push_back(newInput.get());
    newInput.release();
}

void QueryChain::Install( std::auto_ptr<IFilter> newFilter )
{
    filters.push_back(newFilter.get());
    newFilter.release();
}

void QueryChain::Install( std::auto_ptr<IOutput> newOutput )
{
    outputs.push_back(newOutput.get());
    newOutput.release();
}

QueryChain::~QueryChain()
{
    std::for_each(inputs.begin(), inputs.end(), Deleter<IInputProvider *>());
    std::for_each(filters.begin(), filters.end(), Deleter<IFilter *>());
    std::for_each(outputs.begin(), outputs.end(), Deleter<IOutput *>());
}

void QueryChain::Run()
{
    for (std::vector<IInputProvider*>::iterator it = inputs.begin(), end = inputs.end();
        it != end; ++it)
    {
        RunInput(**it);
    }
}

void QueryChain::RunInput( IInputProvider& provider )
{

}

}
