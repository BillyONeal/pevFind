#include <iostream>
#include <algorithm>
#include <memory>
#include <iomanip>
#include <functional>

char const* Name(bool name)
{
    if (name)
    {
        return "negated";
    }
    else
    {
        return "not negated";
    }
}

void Print(bool const* perms, std::size_t const len)
{
	if (len == 0)
	{
		std::cout << "{}\n";
		return;
	}

	std::cout << "{" << Name(perms[0]);
	for (std::size_t idx = 1; idx < len; ++idx)
	{
		std::cout << ", " <<  Name(perms[idx]);
	}
	std::cout << "}\n";
}

std::unique_ptr<bool[]> MakePermutationStart(std::size_t currentIndex, std::size_t maxLen)
{
	std::unique_ptr<bool[]> result(new bool[maxLen]);
	for (std::size_t idx = 0; idx < currentIndex; ++idx)
	{
		result[idx] = false;
	}
	for (std::size_t idx = currentIndex; idx < maxLen; ++idx)
	{
		result[idx] = true;
	}
	return result;
}

int main()
{
	const std::size_t permsCount = 3;
	for (std::size_t idx = 1; idx <= permsCount; idx += 2)
	{
		std::cout << "Iteration for " << idx << " negations.\n";
		auto permutations = MakePermutationStart(idx, permsCount);
		do
		{
			Print(permutations.get(), permsCount);
		} 
		while(std::next_permutation(permutations.get(), permutations.get() + permsCount));
		std::cout << "\n";
	}
}

