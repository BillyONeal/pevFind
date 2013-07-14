//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "pch.hpp"
#include "reportAssembler.hpp"

namespace vFind {

void QueryChain::Install( std::unique_ptr<IInputProvider> newInput )
{
    inputs.emplace_back(std::move(newInput));
}

void QueryChain::Install( std::unique_ptr<IFilter> newFilter )
{
    filters.emplace_back(std::move(newFilter));
}

void QueryChain::Install( std::unique_ptr<IOutput> newOutput )
{
    outputs.emplace_back(std::move(newOutput));
}

void QueryChain::Run()
{
    for (auto it = inputs.begin(), end = inputs.end();
        it != end; ++it)
    {
        RunInput(**it);
    }
}

void QueryChain::RunInput( IInputProvider& provider )
{

}

}
