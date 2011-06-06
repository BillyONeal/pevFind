#include <cassert>
#include <vector>
#include <string>
#include <ostream>
#include <sstream>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <boost/algorithm/string/predicate.hpp>
#include "regscriptCompiler.h"

regscriptCompiler::~regscriptCompiler()
{
    for (std::vector<opCode>::iterator it = parsedResults.begin(); it != parsedResults.end(); it++)
    {
    	free(it->data);
    }
}

bool regscriptCompiler::parse(boost::iterator_range<std::wstring::const_iterator> inputScript)
{
    ansi = true;
    errorCount = 0;
    warningCount = 0;
	fixNull = true;
    lineCount = 1;
    outputText.str(std::wstring());
    begin = current = inputScript.begin();
    end = inputScript.end();
    skipWhitespace();
    parseVersion();
    skipWhitespace();
    for(;current != end && parseKeySpec(); skipWhitespace())
    {
    }
    return true;
}

void regscriptCompiler::skipWhitespace()
{
    for (;current != end; current++)
    {
        switch (*current)
        {
        case L';':
            current = std::find(current, end, L'\n');
			current--;
            break;
        case L'\r':
            current++;
            if (current == end) return;
        case L'\n':
            lineCount++;
            break;
        case L'\t':
        case L' ':
        case 0xFEFF:
            break;
        default:
            return;
        }
    }
}

void regscriptCompiler::parseVersion()
{
    if (boost::algorithm::istarts_with(boost::make_iterator_range(current, end), L"Windows Registry Editor Version 5.00"))
    {
        ansi = false;
        current += 36;
    }
    else if (boost::algorithm::istarts_with(boost::make_iterator_range(current, end), L"REGEDIT4"))
    {
        current += 8;
    }
    else
    {
        outputText << L"Warning: Expected a version declaration on line " << lineCount << L". Defaulting to REGEDIT4." << std::endl;
        warningCount++;
    }
}

bool regscriptCompiler::parseKeySpec()
{
    if (current + 3 >= end) return false; //Keyspec requires at least one [, one ] and one character
    if (*current != L'[')
    {
		outputText << L"Error: Expected a key specification on line " << lineCount << L".\r\n";
		outputText << std::wstring(current, (end-current) > 100? current + 100 : end) << std::endl;
        errorCount++;
        current = std::find(current, end, L'\n');
        return true;
    }
    current++;
    if (*current == L'-') // Delete Key
    {
        current++;
        if (!loadNameAndRoot()) return false;
		pushArgumentlessOpcode(deleteKey);
    }
    else
    {
		if (!loadNameAndRoot()) return false;
		pushArgumentlessOpcode(createKey);
		for(skipWhitespace(); current != end && parseValueSpec(); skipWhitespace()) {};
		pushArgumentlessOpcode(closeKey);
    }
    return true;
}

bool regscriptCompiler::loadNameAndRoot()
{
	using namespace boost;
	opCode rootLoadCode;
	opCode nameLoadCode;
	rootLoadCode.code = loadKeyRoot;
	rootLoadCode.data = malloc(sizeof(HANDLE));
	rootLoadCode.dataLength = sizeof(HANDLE);
	nameLoadCode.code = loadName;
	if (istarts_with(make_iterator_range(current, end), L"HKEY_LOCAL_MACHINE"))
	{
		current += 18;
		*((PHKEY)rootLoadCode.data) = HKEY_LOCAL_MACHINE;
	}
	else if (istarts_with(make_iterator_range(current, end), L"HKLM"))
	{
		current += 4;
		*((PHKEY)rootLoadCode.data) = HKEY_LOCAL_MACHINE;
	}
	else if (istarts_with(make_iterator_range(current, end), L"HKEY_CLASSES_ROOT"))
	{
		current += 17;
		*((PHKEY)rootLoadCode.data) = HKEY_CLASSES_ROOT;
	}
	else if (istarts_with(make_iterator_range(current, end), L"HKCR"))
	{
		current += 4;
		*((PHKEY)rootLoadCode.data) = HKEY_CLASSES_ROOT;
	}
	else if (istarts_with(make_iterator_range(current, end), L"HKEY_CURRENT_USER"))
	{
		current += 17;
		*((PHKEY)rootLoadCode.data) = HKEY_CURRENT_USER;
	}
	else if (istarts_with(make_iterator_range(current, end), L"HKCU"))
	{
		current += 4;
		*((PHKEY)rootLoadCode.data) = HKEY_CURRENT_USER;
	}
	else if (istarts_with(make_iterator_range(current, end), L"HKEY_USERS"))
	{
		current += 10;
		*((PHKEY)rootLoadCode.data) = HKEY_USERS;
	}
	else if (istarts_with(make_iterator_range(current, end), L"HKU"))
	{
		current += 3;
		*((PHKEY)rootLoadCode.data) = HKEY_USERS;
	}
	else if (istarts_with(make_iterator_range(current, end), L"HKEY_PERFORMANCE_DATA"))
	{
		current += 21;
		*((PHKEY)rootLoadCode.data) = HKEY_PERFORMANCE_DATA;
	}
	else if (istarts_with(make_iterator_range(current, end), L"HKEY_PERFORMANCE_TEXT"))
	{
		current += 21;
		*((PHKEY)rootLoadCode.data) = HKEY_PERFORMANCE_TEXT;
	}
	else if (istarts_with(make_iterator_range(current, end), L"HKEY_PERFORMANCE_NLSTEXT"))
	{
		current += 24;
		*((PHKEY)rootLoadCode.data) = HKEY_PERFORMANCE_NLSTEXT;
	}
	else if (istarts_with(make_iterator_range(current, end), L"HKEY_CURRENT_CONFIG"))
	{
		current += 19;
		*((PHKEY)rootLoadCode.data) = HKEY_PERFORMANCE_DATA;
	}
	else if (istarts_with(make_iterator_range(current, end), L"HKEY_DYN_DATA"))
	{
		current += 13;
		*((PHKEY)rootLoadCode.data) = HKEY_DYN_DATA;
	}
	else
	{
		outputText << L"I don't know what root key that is on line " << lineCount << L".\n";
	}
	if (current != end && *current == L'\\')
		current++;
	std::wstring::const_iterator keyNameEnd = getEndOfKeyName();
	if (current == keyNameEnd)
	{
		nameLoadCode.data = NULL;
		nameLoadCode.dataLength = 0;
	}
	else
	{
		nameLoadCode.dataLength = (keyNameEnd - current + 1) * sizeof(wchar_t); //1 for null terminator
		nameLoadCode.data = malloc(nameLoadCode.dataLength);
		*(((wchar_t *) nameLoadCode.data) + (keyNameEnd - current)) = 0; //Null terminator
		std::copy(current, keyNameEnd, ((wchar_t *) nameLoadCode.data));
		current = keyNameEnd;
		if (current != end) current++;
	}
	parsedResults.push_back(rootLoadCode);
	parsedResults.push_back(nameLoadCode);
	current = keyNameEnd + 1;
	return true;
}

void regscriptCompiler::pushArgumentlessOpcode(const std::size_t code)
{
	opCode newCode;
	newCode.code = code;
	newCode.data = NULL;
	newCode.dataLength = 0;
	parsedResults.push_back(newCode);
}

std::wstring::const_iterator regscriptCompiler::getEndOfKeyName()
{
	for (std::wstring::const_iterator keyNameEnd = current; keyNameEnd != end; keyNameEnd++)
	{
		switch (*keyNameEnd)
		{
		case L']':
			if (keyNameEnd +1 == end || *(keyNameEnd + 1) == L'\r' || *(keyNameEnd + 1) == L'\n')
				return keyNameEnd;
			break;
        case L'\r':
			if (keyNameEnd + 1 != end) keyNameEnd++;
        case L'\n':
			outputText << L"Warning: Newline found in key name. Regedit does not allow this, though I am. Line: " << lineCount << L".\r\n";
            lineCount++;
			warningCount++;
            break;
		}
	}
	return end;
}

bool regscriptCompiler::parseValueSpec()
{
	using namespace boost;
	opCode nameLoader;
	nameLoader.code = loadName;
	switch(*current)
	{
	case L'@':
		nameLoader.data = NULL;
		nameLoader.dataLength = 0;
		current++;
		break;
	case L'"':
		parseCString((wchar_t **)&nameLoader.data, &nameLoader.dataLength);
		break;
	default:
		return false;
	}
	parsedResults.push_back(nameLoader);
	current++; //Skip over = sign
	DWORD type = NULL;
	if (*current == L'-') // Delete key
	{
		pushArgumentlessOpcode(deleteValue);
		current = std::find(current, end, L'\n');
		return true;
	}
	else if (*current == L'"')
	{
		type = REG_SZ;
		opCode stringData;
		stringData.code = loadData;
		parseCString((wchar_t**) &stringData.data, &stringData.dataLength);
		current = std::find(current, end, L'\n');
		parsedResults.push_back(stringData);
	}
	else if (istarts_with(make_iterator_range(current, end), L"dword"))
	{
		type = REG_DWORD;
		current += 5;
		if (current != end && *current == L':') current++;
		opCode dwordCode;
		dwordCode.code = loadData;
		dwordCode.dataLength = sizeof(DWORD);
		dwordCode.data = malloc(sizeof(DWORD));
		*((DWORD *) dwordCode.data) = parseDword();
		current = std::find(current, end, L'\n');
		parsedResults.push_back(dwordCode);
	}
	else if (istarts_with(make_iterator_range(current, end), L"hex"))
	{
		current += 3;
		if (*current == L'(')
		{
			current++;
			type = parseDword();
			if (current != end && *current == L')') current++;
		}
		else
		{
			type = REG_NONE;
		}
		if (current != end && *current == L':') current++;
		opCode hexData;
		hexData.code = loadData;
		parseHexValueSpec((char  **) &hexData.data, &hexData.dataLength);
		if (ansi && type == 1 || type == 2 || type == 7) //Convert to Unicode if needed.
		{
			int newLength =
				MultiByteToWideChar(CP_ACP, NULL, (char *)hexData.data, hexData.dataLength, NULL, NULL);
			void * unicodeData = malloc(newLength * sizeof(wchar_t));
			MultiByteToWideChar(CP_ACP, NULL,  (char *)hexData.data, hexData.dataLength, (wchar_t *)unicodeData, newLength);
			free(hexData.data);
			hexData.dataLength = newLength * sizeof(wchar_t);
			hexData.data = unicodeData;
		}
		//Account for idiots who can't put NULLs in their regscripts.
		if (fixNull && (type == 1 || type == 2 || type == 7) && (hexData.dataLength < 2 || memcmp(((char *)hexData.data) + hexData.dataLength - 2, "\0\0", 2)))
		{
			char * temp = (char *) malloc(hexData.dataLength + 2);
			memcpy(temp, hexData.data, hexData.dataLength);
			*(temp + hexData.dataLength) = 0;
			*(temp + hexData.dataLength + 1) = 0;
			hexData.dataLength += 2;
			free(hexData.data);
			hexData.data = temp;
		}
		if (fixNull && type == 7 && (hexData.dataLength < 2 || memcmp(((char *)hexData.data) + hexData.dataLength - 4, "\0\0", 2)))
		{
			char * temp = (char *) malloc(hexData.dataLength + 2);
			memcpy(temp, hexData.data, hexData.dataLength);
			*(temp + hexData.dataLength) = 0;
			*(temp + hexData.dataLength + 1) = 0;
			hexData.dataLength += 2;
			free(hexData.data);
			hexData.data = temp;
		}
		parsedResults.push_back(hexData);
	}
	else
	{
		outputText << L"Unrecognised valueSpec on line " << lineCount << L". (Did you forget a -?) Value ignored." << std::endl;
		current = std::find(current, end, L'\n');
		parsedResults.pop_back(); // Remove the name load instruction.
		return true;
	}
	opCode setValueCode;
	setValueCode.code = setValue;
	setValueCode.data = malloc(sizeof(DWORD));
	*((DWORD *)setValueCode.data) = type;
	setValueCode.dataLength = sizeof(DWORD);
	parsedResults.push_back(setValueCode);
	return true;
}

void regscriptCompiler::parseCString(wchar_t ** str, std::size_t *length)
{
	*str = NULL;
	*length = 0;
	std::wstring::const_iterator cur;
	if (current + 2 > end) return; //length guard (Always need two "s for a c string)
	if (*current == L'"') current++;
	//Calculate required length
	for(cur = current; cur != end && *cur != L'"'; cur++, (*length)++)
	{
		if (*cur == L'\\')
			cur++;
		if (*cur == L'\n')
		{
			lineCount++;
			warningCount++;
			outputText << L"Warning: Newline found in CString literal on line " << lineCount << ".\r\n";
		}
	}
	if (*length == 0) return;
	if ((*cur) != L'"') return; //Reached end of string, not end of item.
	*length = (*length + 1) * sizeof(wchar_t);
	*str = (wchar_t *) malloc(*length);
	wchar_t *copyTo = *str;
	std::size_t idx;
	for (idx = 0; idx * sizeof(wchar_t) != *length; idx++, current++)
	{
		if (*current != L'\\')
			*(copyTo + idx) = *current;
		else
		{
			current++;
			switch(*current)
			{
			case L'n':
				*(copyTo + idx) = L'\n';
				break;
			case L't':
				*(copyTo + idx) = L'\t';
				break;
			case L'0':
				*(copyTo + idx) = L'\0';
				break;
			case L'b':
				*(copyTo + idx) = L'\b';
				break;
			default:
				*(copyTo + idx) = *current;
			}
		}
	}
	*(copyTo + idx - 1) = NULL;
	if (*current == L'"') current++;
}
DWORD regscriptCompiler::parseDword()
{
	DWORD result = 0;
	for (; current != end; current++)
	{
		if (*current <= L'9' && *current >= L'0')
		{
			result = (result << 4) | *current - L'0';
		}
		else if (*current <= L'F' && *current >= L'A')
		{
			result = (result << 4) | (*current - L'A' + 10);
		}
		else if (*current <= L'f' && *current >= L'a')
		{
			result = (result << 4) | (*current - L'a' + 10);
		}
		else
			break;
	}
	return result;
}

void regscriptCompiler::parseHexValueSpec(char ** str, std::size_t *length)
{
	std::vector<unsigned char> tempBuffer;
	for(; current != end; current++)
	{
		switch(*current)
		{
		case L'[':
		case L'"':
		case L'\'':
			goto writeClientBuffer;
        case L';':
            current = std::find(current, end, L'\n');
            break;
        case L'\r':
            current++;
            if (current == end) goto writeClientBuffer;
        case L'\n':
            lineCount++;
            break;
        case L'\t':
        case L' ':
        case 0xFEFF:
            break;
		case L'0':
			tempBuffer.push_back(0);
			break;
		case L'1':
			tempBuffer.push_back(1);
			break;
		case L'2':
			tempBuffer.push_back(2);
			break;
		case L'3':
			tempBuffer.push_back(3);
			break;
		case L'4':
			tempBuffer.push_back(4);
			break;
		case L'5':
			tempBuffer.push_back(5);
			break;
		case L'6':
			tempBuffer.push_back(6);
			break;
		case L'7':
			tempBuffer.push_back(7);
			break;
		case L'8':
			tempBuffer.push_back(8);
			break;
		case L'9':
			tempBuffer.push_back(9);
			break;
		case L'A':
		case L'a':
			tempBuffer.push_back(10);
			break;
		case L'B':
		case L'b':
			tempBuffer.push_back(11);
			break;
		case L'C':
		case L'c':
			tempBuffer.push_back(12);
			break;
		case L'D':
		case L'd':
			tempBuffer.push_back(13);
			break;
		case L'E':
		case L'e':
			tempBuffer.push_back(14);
			break;
		case L'F':
		case L'f':
			tempBuffer.push_back(15);
			break;
		}
	}
writeClientBuffer:
	if (tempBuffer.size() & 0x1)
	{
		outputText << L"Warning: Invalid hex specification ending on line " << lineCount << L". Hex specs must be in groups of two";
		outputText << L"characters. 4 bits do not make a byte ;). Last character will be ignored.\n";
		tempBuffer.pop_back();
	}
	*length = tempBuffer.size() / 2;
	*str = (char *)malloc(*length);
	std::vector<unsigned char>::const_iterator bufferIt = tempBuffer.begin();
	std::size_t stringPos = 0;
	for(; bufferIt != tempBuffer.end(); bufferIt+=2, stringPos++)
	{
		*(*str + stringPos) = (*bufferIt << 4) | *(bufferIt + 1);
	}
}

wchar_t getHexChar(char toConvert)
{
	static const wchar_t hexChars[] = L"0123456789ABCDEF";
	toConvert &= 0x0F;
	return hexChars[toConvert];
}

void regscriptCompiler::printASM()
{
	for(std::vector<opCode>::const_iterator it = parsedResults.begin(); it != parsedResults.end(); it++)
	{
		switch(it->code)
		{
		case loadName:
			outputText << L"LDNAME: " << std::wstring((const wchar_t *) it->data, ((const wchar_t *) it->data) + ((it->dataLength / sizeof(wchar_t)))) << std::endl;
			break;
		case loadKeyRoot:
			outputText << L"LDROOT: ";
			switch((unsigned long)(*((HANDLE *) it->data)))
			{
			case HKEY_LOCAL_MACHINE:
				outputText << L"HKEY_LOCAL_MACHINE";
				break;
			case HKEY_CLASSES_ROOT:
				outputText << L"HKEY_CLASSES_ROOT";
				break;
			case HKEY_CURRENT_USER:
				outputText << L"HKEY_CURRENT_USER";
				break;
			case HKEY_USERS:
				outputText << L"HKEY_USERS";
				break;
			default:
				outputText << L"HKEY_BUG!!";
			}
			outputText << std::endl;
			break;
		case loadData:
			outputText << L"LDDATA: ";
			if (it->dataLength)
			{
				outputText << getHexChar(*((const char*) it->data) >> 4);
				outputText << getHexChar(*((const char*) it->data));
			}
			for (std::size_t idx = 1; idx < it->dataLength; idx++)
			{
				outputText << L", ";
				outputText << getHexChar(*(((const char*) it->data) + idx) >> 4);
				outputText << getHexChar(*(((const char*) it->data) + idx));
			}
			outputText << std::endl;
			break;
		case createKey:
			outputText << L"CREATE:\n";
			break;
		case closeKey:
			outputText << L"KCLOSE:\n";
			break;
		case deleteKey:
			outputText << L"KEYDEL:\n";
			break;
		case setValue:
			outputText << L"SETVAL: ";
			switch (*(DWORD *) it->data)
			{
			case REG_NONE:
				outputText << L"REG_NONE";
				break;
			case REG_SZ:
				outputText << L"REG_SZ";
				break;
			case REG_EXPAND_SZ:
				outputText << L"REG_EXPAND_SZ";
				break;
			case REG_BINARY:
				outputText << L"REG_BINARY";
				break;
			case REG_DWORD_LITTLE_ENDIAN:
				outputText << L"REG_DWORD_LITTLE_ENDIAN";
				break;
			case REG_DWORD_BIG_ENDIAN:
				outputText << L"REG_DWORD_BIG_ENDIAN";
				break;
			case REG_LINK:
				outputText << L"REG_LINK";
				break;
			case REG_MULTI_SZ:
				outputText << L"REG_MULTI_SZ";
				break;
			case REG_RESOURCE_LIST:
				outputText << L"REG_RESOURCE_LIST";
				break;
			case REG_FULL_RESOURCE_DESCRIPTOR:
				outputText << L"REG_FULL_RESOURCE_DESCRIPTOR";
				break;
			case REG_RESOURCE_REQUIREMENTS_LIST:
				outputText << L"REG_RESOURCE_REQUIREMENTS_LIST";
				break;
			case REG_QWORD:
				outputText << L"REG_QWORD";
				break;
			}
			outputText << L" (" << *((DWORD *) it->data) << L")\n";
			break;
		case deleteValue:
			outputText << L"DELVAL:\n";
			break;
		default:
			outputText << L"BUG!!!\n";
		}
	}
}

void recursiveKeyDelete(HKEY root, const wchar_t * name)
{
	wchar_t tempSubKey[256];
	DWORD index = 0;
	DWORD garbage = 256;
	HKEY keyToNuke;
	if (RegOpenKeyEx(root, name, NULL, KEY_ENUMERATE_SUB_KEYS, &keyToNuke) != ERROR_SUCCESS)
		return;
	while(!RegEnumKeyEx(keyToNuke, index++, tempSubKey, &garbage, NULL, NULL, NULL, NULL))
	{
		recursiveKeyDelete(keyToNuke, tempSubKey);
		garbage = 256;
	}
	RegCloseKey(keyToNuke);
	RegDeleteKey(root, name);
}

void regscriptCompiler::execute()
{
	HKEY hRun = (HKEY) INVALID_HANDLE_VALUE;
	HKEY hRoot = (HKEY) INVALID_HANDLE_VALUE;
	LPBYTE dataPtr = NULL;
	DWORD dataPtrLen = NULL;
	const wchar_t * namePtr = NULL;
	for(std::vector<opCode>::const_iterator it = parsedResults.begin(); it != parsedResults.end(); it++)
	{
		switch(it->code)
		{
		case loadName:
			namePtr = static_cast<const wchar_t *>( it->data );
			break;
		case loadKeyRoot:
			hRoot = *(static_cast<PHKEY>( it->data ));
			break;
		case loadData:
			dataPtr = static_cast<LPBYTE>( it->data );
			dataPtrLen = (DWORD)it->dataLength;
			break;
		case createKey:
			if (RegCreateKeyEx(hRoot, namePtr, NULL, NULL, NULL, KEY_SET_VALUE, NULL, &hRun, NULL) != ERROR_SUCCESS)
				hRun = (HKEY) INVALID_HANDLE_VALUE;
			break;
		case closeKey:
			if (hRun != INVALID_HANDLE_VALUE)
			{
				RegCloseKey(hRun);
				hRun = (HKEY) INVALID_HANDLE_VALUE;
			}
			break;
		case deleteKey:
			recursiveKeyDelete(hRoot, namePtr);
			break;
		case setValue:
			if (hRun != INVALID_HANDLE_VALUE)
				RegSetValueEx(hRun, namePtr, NULL, *(static_cast<DWORD *>(it->data)), dataPtr, dataPtrLen);
			break;
		case deleteValue:
			if (hRun != INVALID_HANDLE_VALUE)
				RegDeleteValue(hRun, namePtr);
			break;
		}
	}
}

bool regscriptCompiler::succeeded()
{
	if (errorCount) return false;
	if (warningCount) return false;
	return true;
}


