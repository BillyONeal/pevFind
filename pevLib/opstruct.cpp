#include "pch.hpp"
#include <string>
#include <boost/algorithm/string/replace.hpp>
#include "opstruct.h"

std::wstring operation::debugTreeInternal(const std::wstring& curOp) const
{
	std::wstring dbgMsg(L"- ");
	dbgMsg.append(curOp);
	dbgMsg.append(L"\r\n");
	dbgMsg.append(operandA->debugTree());
	dbgMsg.append(L"\r\n");
	dbgMsg.append(operandB->debugTree());
	boost::algorithm::replace_all(dbgMsg, L"\r\n", L"\r\n   ");
	return dbgMsg;
}
void operation::reorderTree()
{
	if (operandA->getPriorityClass() > operandB->getPriorityClass())
		std::swap(operandA,operandB);
	operandA->reorderTree();
	operandB->reorderTree();
};
unsigned __int32 operation::getPriorityClass() const
{
	unsigned __int32 classA, classB;
	classA = operandA->getPriorityClass();
	classB = operandB->getPriorityClass();
	if (classA > classB)
		return classA;
	return classB;

};
operation::operation(boost::shared_ptr<criterion> a, boost::shared_ptr<criterion> b) : operandA(a), operandB(b)
{}
BOOL andAndClass::include(FileData &file) const
{
	if (this->operandA->include(file) && this->operandB->include(file))
		return true;
	return false;
}
unsigned int andAndClass::directoryCheck(const std::wstring& directory) const
{
	unsigned int resA, resB;
	resA = operandA->directoryCheck(directory);
	resB = operandB->directoryCheck(directory);
	if (resA == DIRECTORY_DONTCARE && resB == DIRECTORY_DONTCARE)
		return DIRECTORY_DONTCARE;
	if (resA == DIRECTORY_DONTCARE)
		return resB;
	if (resB == DIRECTORY_DONTCARE)
		return resA;
	return resA && resB;
}
andAndClass::andAndClass(boost::shared_ptr<criterion> a, boost::shared_ptr<criterion> b) : operation(a, b)
{}
std::wstring andAndClass::debugTree() const
{
	return debugTreeInternal(L"AND");
}
BOOL orAndClass::include(FileData &file) const
{
	if (this->operandA->include(file))
		return true;
	if (this->operandB->include(file))
		return true;
	return false;
}
unsigned int orAndClass::directoryCheck(const std::wstring& directory) const
{
	unsigned int resA, resB;
	resA = operandA->directoryCheck(directory);
	resB = operandB->directoryCheck(directory);
	if (resA == DIRECTORY_DONTCARE && resB == DIRECTORY_DONTCARE)
		return DIRECTORY_DONTCARE;
	if (resA == DIRECTORY_DONTCARE)
		return resB;
	if (resB == DIRECTORY_DONTCARE)
		return resA;
	return resA || resB;
}
orAndClass::orAndClass(boost::shared_ptr<criterion> a, boost::shared_ptr<criterion> b) : operation(a, b)
{}
std::wstring orAndClass::debugTree() const
{
	return debugTreeInternal(L"OR");
}
BOOL xorAndClass::include(FileData &file) const
{
	BOOL resA = this->operandA->include(file);
	BOOL resB = this->operandB->include(file);
	return (resA || resB) && !(resA && resB);
}
unsigned int xorAndClass::directoryCheck(const std::wstring& directory) const
{
	unsigned int resA, resB;
	resA = operandA->directoryCheck(directory);
	resB = operandB->directoryCheck(directory);
	if (resA == DIRECTORY_DONTCARE && resB == DIRECTORY_DONTCARE)
		return DIRECTORY_DONTCARE;
	if (resA == DIRECTORY_DONTCARE)
		return resB;
	if (resB == DIRECTORY_DONTCARE)
		return resA;
	return (resA || resB) && !(resA && resB);
}
xorAndClass::xorAndClass(boost::shared_ptr<criterion> a, boost::shared_ptr<criterion> b) : operation(a, b)
{}
std::wstring xorAndClass::debugTree() const
{
	return debugTreeInternal(L"XOR");
}
void notAndClass::reorderTree()
{
	operand->reorderTree();
}
unsigned __int32 notAndClass::getPriorityClass() const
{
	return operand->getPriorityClass();
}
BOOL notAndClass::include(FileData &file) const
{
	return !(operand->include(file));
}
unsigned int notAndClass::directoryCheck(const std::wstring& directory) const
{
	switch(operand->directoryCheck(directory))
	{
	case DIRECTORY_DONTCARE:
	case DIRECTORY_INCLUDE:
		return DIRECTORY_DONTCARE;
	case DIRECTORY_EXCLUDE:
		return DIRECTORY_INCLUDE;
	}
	return DIRECTORY_DONTCARE;
}
notAndClass::notAndClass(boost::shared_ptr<criterion> a) : operand(a)
{}
std::wstring notAndClass::debugTree() const
{
	std::wstring dbgMsg(L"- NOT\r\n");
	dbgMsg.append(operand->debugTree());
	boost::algorithm::replace_all(dbgMsg, L"\r\n", L"\r\n   ");
	return dbgMsg;
}
void bracketClass::reorderTree()
{
	std::sort(expr.begin(), expr.end(),criterionByPriorityClass);
}
unsigned __int32 bracketClass::getPriorityClass() const
{
	//Create a vector with the same size as the expression
	std::vector<__int32> priNumbers(expr.size());
	//Transform the expression items into priority numbers
	std::transform(expr.begin(), expr.end(), priNumbers.begin(), getPriorityFunctor);
	return *(std::max_element(priNumbers.begin(), priNumbers.end()));
}
BOOL bracketClass::include(FileData &file) const
{
	for (register unsigned int idx = 0; idx < expr.size(); idx++)
		if (!expr[idx]->include(file))
			return FALSE;
	return TRUE;
}
unsigned int bracketClass::directoryCheck(const std::wstring& directory)
{
	bool allDontcares = true;
	for (register unsigned int idx = 0; idx < expr.size(); idx++)
	{
		if (!expr[idx]->directoryCheck(directory))
			return DIRECTORY_EXCLUDE;
		if (expr[idx]->directoryCheck(directory) == DIRECTORY_INCLUDE)
			allDontcares = false;
	}
	if (allDontcares)
		return DIRECTORY_DONTCARE;
	else
		return DIRECTORY_INCLUDE;
}
bracketClass::bracketClass(std::vector<boost::shared_ptr<criterion> > exprA): expr(exprA)
{ 
	assert(expr.size() > 1);
}
std::wstring bracketClass::debugTree() const
{
	std::wstring dbgMsg(L"- BRACKET\r\n");
	for (std::vector<boost::shared_ptr<criterion> >::const_iterator it = expr.begin(); it != expr.end(); it++)
	{
		dbgMsg.append((*it)->debugTree());
		if (it + 1 != expr.end())
			dbgMsg.append(L"\r\n");
	}
	boost::algorithm::replace_all(dbgMsg, L"\r\n", L"\r\n   ");
	return dbgMsg;
}
void ifClass::reorderTree()
{
	condVal->reorderTree();
	tVal->reorderTree();
	if (fVal)
		fVal->reorderTree();
}
unsigned __int32 ifClass::getPriorityClass() const
{
	unsigned __int32 condValClass, tValClass, fValClass;
	condValClass = condVal->getPriorityClass();
	tValClass = tVal->getPriorityClass();
	if (fVal)
		fValClass = fVal->getPriorityClass();
	else
		fValClass = 0; //Min value means it won't matter in following comparison.
	if (condValClass >= tValClass)
	{
		if (condValClass >= fValClass)
			return condValClass;
		else
			return fValClass;
	} else
	{
		if (tValClass >= fValClass)
			return tValClass;
		else
			return fValClass;
	}
}
BOOL ifClass::include(FileData &file) const
{
	if (condVal->include(file))  //If we have this item
	{
		return tVal->include(file); //Return valueIfTrue
	} else                      //Otherwise
	{
		if (!fVal)              //If there is no ELSE item,
			return true;        //assume true
		return fVal->include(file); //Otherwise return value of the
		                        //else part of the node.
	}
}
unsigned int ifClass::directoryCheck(const std::wstring& directory) const
{
	if (condVal->directoryCheck(directory))    //See rationale in include() method
	{
		return tVal->directoryCheck(directory);
	} else
	{
		if (!fVal)
			return DIRECTORY_DONTCARE;
		return fVal->directoryCheck(directory);
	}
}
ifClass::ifClass(boost::shared_ptr<criterion> condition,boost::shared_ptr<criterion> valueIfTrue,boost::shared_ptr<criterion> valueIfFalse)
	: condVal(condition), tVal(valueIfTrue), fVal(valueIfFalse)
{}
std::wstring ifClass::debugTree() const
{
	std::wstring result;

	std::wstring condMsg(L"- IF\r\n");
	condMsg.append(condVal->debugTree());
	boost::algorithm::replace_all(condMsg, L"\r\n", L"\r\n   ");
	result.assign(condMsg);
	result.append(L"\r\n");

	std::wstring thenMsg(L"- THEN\r\n");
	thenMsg.append(tVal->debugTree());
	boost::algorithm::replace_all(thenMsg, L"\r\n", L"\r\n   ");
	result.append(thenMsg);
	
	std::wstring elseMsg;
	if (fVal)
	{
		result.append(L"\r\n");
		elseMsg.assign(L"- ELSE\r\n");
		elseMsg.append(fVal->debugTree());
		boost::algorithm::replace_all(elseMsg, L"\r\n", L"\r\n   ");
		result.append(elseMsg);
	}

	return result;
}

void operation::makeNonRecursive()
{
	operandA->makeNonRecursive();
	operandB->makeNonRecursive();
}

void notAndClass::makeNonRecursive()
{
	operand->makeNonRecursive();
}

void bracketClass::makeNonRecursive()
{
	for (std::vector<boost::shared_ptr<criterion> >::iterator it = expr.begin(); it != expr.end(); it++)
	{
		(*it)->makeNonRecursive();
	}
}

void ifClass::makeNonRecursive()
{
	condVal->makeNonRecursive();
	tVal->makeNonRecursive();
	if (fVal.get())
		fVal->makeNonRecursive();
}
