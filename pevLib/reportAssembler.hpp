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
    std::vector<std::unique_ptr<IInputProvider>> inputs;
    std::vector<std::unique_ptr<IFilter>> filters;
    std::vector<std::unique_ptr<IOutput>> outputs;

    void RunInput(IInputProvider&);

public:
    void Install(std::unique_ptr<IInputProvider> newInput);
    void Install(std::unique_ptr<IFilter> newFilter);
    void Install(std::unique_ptr<IOutput> newOutput);

    void Run();
};

}
