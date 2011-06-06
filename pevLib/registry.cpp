#include <string>
#include <windows.h>
#include "registry.h"

namespace registry
{

registryKey::registryKey(const std::wstring &path)
{
	std::wstring::const_iterator pathStart = getKeyRoot(path.begin(), path.end(), root);
	std::wstring::const_iterator pathEnd = path.end();
	if (root == (HKEY)INVALID_HANDLE_VALUE)
		throw registryException("Invalid root key specified.");
	//Handle cases where there is still the trailing ] in the key name.
	if (*(pathEnd - 1) == L']')
		pathEnd--;
	path_ = std::wstring(pathStart, pathEnd);
	LONG createExResult = RegCreateKeyEx(
			root,                      //hKey 
			path_.c_str(),             //lpSubKey
			NULL,                      //Reserved
			NULL,                      //lpClass
			0,                         //dwOptions
			KEY_ALL_ACCESS,            //samDesired
			NULL,                      //lpSecurityAttributes
			&hWin32,                   //phkResult
			NULL);                     //lpdwDisposition
	if (createExResult != ERROR_SUCCESS)
	{
		throw registryException("Could not open the specified key.");
	}
}

registryKey::registryKey(const registryKey& lhs) : root(lhs.root), path_(lhs.path_)
{
	BOOL result;
	result = DuplicateHandle
	(
		GetCurrentProcess(),   //hSourceProcessHandle
		lhs.hWin32,            //hSourceHandle
        GetCurrentProcess(),   //hTargetProcessHandle
		(LPHANDLE)&hWin32,     //lpTargetHandle
		NULL,                  //dwDesiredAccess
		false,                 //bInheritHandle
		DUPLICATE_SAME_ACCESS  //dwOptions
	);
	if(!result)
		throw registryException("Could not duplicate registry handle for copying key.");
}

registryKey::~registryKey()
{
	RegCloseKey(hWin32);
}

registryKey& registryKey::operator=(const registryKey& lhs)
{
	RegCloseKey(hWin32);
	BOOL result;
	result = DuplicateHandle
	(
		GetCurrentProcess(),   //hSourceProcessHandle
		lhs.hWin32,            //hSourceHandle
        GetCurrentProcess(),   //hTargetProcessHandle
		(LPHANDLE)&hWin32,     //lpTargetHandle
		NULL,                  //dwDesiredAccess
		false,                 //bInheritHandle
		DUPLICATE_SAME_ACCESS  //dwOptions
	);
	if(!result)
		throw registryException("Could not duplicate registry handle for copying key.");
	path_ = lhs.path_;
	root = lhs.root;
	return *this;
}

registryValue registryKey::operator[](const std::wstring &lhs)
{
	return registryValue(*this, lhs);
}

registryValue::registryValue(registry::registryKey &parent, const std::wstring &name) : name_(name), parent_(&parent)
{
}

void registryValue::constructDataBlock(DWORD * type, std::vector<char>& dataBlock) const
{
	LONG result;
	DWORD vecSize = 0;
	do
	{
		dataBlock.resize(vecSize);
		result = RegQueryValueEx
		(
			parent_->hWin32,          //hkey
			name_.c_str(),            //lpValueName
			0,                        //lpReserved
			type,                     //lpType
			(LPBYTE)&dataBlock[0],    //lpData
			&vecSize                  //lpcbData
		);
	} while(result == ERROR_MORE_DATA);
	if (result != ERROR_SUCCESS)
		throw registryException("Could not get data for value in registry.");
}

std::wstring registryValue::displayString(const std::vector<char> &dataBlock) const
{
	wchar_t const * start = reinterpret_cast<wchar_t const *>(&dataBlock[0]);
	wchar_t const * end = reinterpret_cast<wchar_t const *>(&dataBlock[dataBlock.size() - sizeof(wchar_t)]);
	std::wstring result;
	result.reserve(end - start + 1);
	for(wchar_t const *curChar = start; curChar != end; curChar++)
	{
		switch(*curChar)
		{
		case L'\a':
			result.append(L"\\a");
			break;
		case L'\b':
			result.append(L"\\b");
			break;
		case L'\f':
			result.append(L"\\f");
			break;
		case L'\n':
			result.append(L"\\n");
			break;
		case L'\r':
			result.append(L"\\r");
			break;
		case L'\t':
			result.append(L"\\t");
			break;
		case L'\v':
			result.append(L"\\v");
			break;
		case L'\0':
			result.append(L"\\0");
			break;
		default:
			result.push_back(*curChar);
		}
	}
	return result;
}

registryValue::registryValue(const registryValue& rhs): name_(rhs.name_), parent_(rhs.parent_)
{
}

registryValue& registryValue::operator=(const registryValue& rhs)
{
	name_ = rhs.name_;
	parent_ = rhs.parent_;
	return *this;
}

void registryValue::setString(const std::wstring & toSet)
{
	LSTATUS result;
	result = RegSetValueEx(parent_->hWin32, name_.c_str(), 0, REG_SZ, (const LPBYTE)toSet.c_str(), (toSet.length() + 1) * sizeof(wchar_t));
	if (result != ERROR_SUCCESS)
		throw registryException("Failed to set value.");
}

std::wstring registryValue::displayHex(const std::vector<char> &dataBlock, DWORD type)
{
	const wchar_t hexChars[] = L"0123456789ABCDEF";
	wchar_t buffer[20];
	swprintf_s(buffer, 20, L"hex(%I32d): ", buffer);
	std::wstring result(buffer);
	std::vector<char>::const_iterator it = dataBlock.begin();
	result.push_back(hexChars[(*it & 0xF0) >> 4]);
	result.push_back(hexChars[(*it & 0x0F)]);
	it++;
	if (it == dataBlock.end())
		return result;
	for(; it != dataBlock.end(); it++)
	{
		result.push_back(L',');
		result.push_back(L' ');
		result.push_back(hexChars[(*it & 0xF0) >> 4]);
		result.push_back(hexChars[(*it & 0x0F)]);
	}
	return result;
}

std::wstring registryValue::asString()
{
	DWORD type;
	std::vector<char> data;
	constructDataBlock(&type, data);
	switch(type)
	{
	case REG_SZ:
	case REG_EXPAND_SZ:
	case REG_LINK:
		//Remove the null at the end if it exists.
		if (*(reinterpret_cast<wchar_t *>(&(data[data.size()])) - 1))
		{
			for (int i = 0; i < sizeof(wchar_t); i++)
				data.pop_back();
		}
	case REG_MULTI_SZ:
		return displayString(data);
		break;
	case REG_DWORD_LITTLE_ENDIAN:
		if (data.size() != sizeof(DWORD))
		{
			displayHex(data, type);
		}
		else
		{
			wchar_t toDisplay[20];
			DWORD * ptr = reinterpret_cast<DWORD *>(&data[0]);
			DWORD value = *ptr;
			swprintf_s(toDisplay, 20, L"%ld", value);
			return toDisplay;
		}
		break;
	case REG_DWORD_BIG_ENDIAN:
		if (data.size() != sizeof(DWORD))
		{
			displayHex(data, type);
		}
		else
		{
			wchar_t toDisplay[20];
			DWORD * ptr = reinterpret_cast<DWORD *>(&data[0]);
			DWORD value = *ptr;
			DWORD valueFlipped = value << 24;
			value = _byteswap_ulong(value);
			swprintf_s(toDisplay, 20, L"%ld", value);
			return toDisplay;
		}
		break;
	case REG_QWORD_LITTLE_ENDIAN:
		if (data.size() != sizeof(DWORD))
		{
			displayHex(data, type);
		}
		else
		{
			wchar_t toDisplay[40];
			DWORD * ptr = reinterpret_cast<DWORD *>(&data[0]);
			DWORD value = *ptr;
			swprintf_s(toDisplay, 40, L"%I64d", value);
			return toDisplay;
		}
		break;
	default:
		displayHex(data, type);
		break;
	}
	throw registryException("Unknown datatype encountered for toString on a registry value.");
}

std::wstring registryValue::operator()()
{
	return asString();
}

registryValue& registryValue::operator=(const std::wstring& rhs)
{
	setString(rhs);
	return *this;
}

DWORD registryValue::getType()
{
	DWORD value;
	LSTATUS errorCheck;
	errorCheck = RegQueryValueEx(parent_->hWin32, name_.c_str(), 0, &value, NULL, NULL);
	if (errorCheck != ERROR_SUCCESS)
		throw registryException("Couldn't get the type of the requested key.");
	return value;
}

std::vector<char> registryValue::getRawDataBlock()
{
	DWORD garbage;
	std::vector<char> result;
	constructDataBlock(&garbage, result);
	return result;
}

const std::wstring& registryValue::getName()
{
	return name_;
}

registryKey& registryValue::getParent()
{
	return *parent_;
}

bool registryValue::asDword(DWORD * result)
{
	std::vector<char> dataBlock;
	DWORD type;
	constructDataBlock(&type, dataBlock);
	switch(type)
	{
	case REG_SZ:
	case REG_MULTI_SZ:
		{
			//Just being safe with the extra NULL
			dataBlock.push_back('\0');
			dataBlock.push_back('\0');
			wchar_t * str = reinterpret_cast<wchar_t *>(&dataBlock[0]);
			swscanf_s(str, L"%ld", result);
		}
	case REG_DWORD:
		{
			if (dataBlock.size() != sizeof(DWORD))
				return false;
			DWORD * ptr = reinterpret_cast<DWORD *>(&dataBlock[0]);
			*result = *ptr;
		}
	default:
		return false;
	}
	return true;
}

void registryValue::setDword(DWORD toSet)
{
	LSTATUS result;
	result = RegSetValueEx(parent_->hWin32, name_.c_str(), 0, REG_DWORD, (const LPBYTE)&toSet, sizeof(DWORD));
	if (result != ERROR_SUCCESS)
		throw registryException("Failed to set value.");
}

bool registryValue::asQword(__int64 * result)
{
	std::vector<char> dataBlock;
	DWORD type;
	constructDataBlock(&type, dataBlock);
	switch(type)
	{
	case REG_SZ:
	case REG_MULTI_SZ:
		{
			//Just being safe with the extra NULL
			dataBlock.push_back('\0');
			dataBlock.push_back('\0');
			wchar_t * str = reinterpret_cast<wchar_t *>(&dataBlock[0]);
			swscanf_s(str, L"%I64d", result);
		}
	case REG_DWORD:
		{
			if (dataBlock.size() != sizeof(DWORD))
				return false;
			DWORD * ptr = reinterpret_cast<DWORD *>(&dataBlock[0]);
			*result = *ptr;
		}
	case REG_QWORD:
		{
			if (dataBlock.size() != sizeof(__int64))
				return false;
			__int64 * ptr = reinterpret_cast<__int64 *>(&dataBlock[0]);
			*result = *ptr;
		}
	default:
		return false;
	}
	return true;
}

void registryValue::setQword(__int64 toSet)
{
	LSTATUS result;
	result = RegSetValueEx(parent_->hWin32, name_.c_str(), 0, REG_DWORD, (const LPBYTE)&toSet, sizeof(__int64));
	if (result != ERROR_SUCCESS)
		throw registryException("Failed to set value.");
}

#define STATE(x) regStateMachine_##x:
#define NEXTSTATE(x) goto regStateMachine_##x
#define NEXTADVANCE(x) begin++; if (begin == end) return end; goto regStateMachine_##x
std::wstring::const_iterator getKeyRoot(std::wstring::const_iterator begin, std::wstring::const_iterator end, HKEY &result)
{
/*
 * This is a state machine which is the most efficient solution to recognise the following strings:
 * HKCC
 * HKCR
 * HKCU
 * HKDD
 * HKEY_CLASSES_ROOT
 * HKEY_CURRENT_CONFIG
 * HKEY_CURRENT_USER
 * HKEY_DYN_DATA
 * HKEY_LOCAL_MACHINE
 * HKEY_PERFORMANCE_DATA
 * HKEY_PERFORMANCE_NLSTEXT
 * HKEY_PERFORMANCE_TEXT
 * HKEY_USERS
 * HKLM
 * HKPD
 * HKPN
 * HKPT
 * HKU
 */
	result = (HKEY)INVALID_HANDLE_VALUE;
	if (*begin == L'[')
		NEXTADVANCE( keyStart );
	NEXTSTATE( keyStart );
	STATE( keyStart )
	{
		switch(*begin)
		{
		case L'H':
		case L'h':
			NEXTADVANCE( H );
		default:
			return end;
		}
	}
	STATE( H )
	{
		switch(*begin)
		{
		case L'K':
		case L'k':
			NEXTADVANCE( HK );
		default:
			return end;
		}
	}
	STATE( HK )
	{
		switch(*begin)
		{
		case L'C':
		case L'c':
			NEXTADVANCE( HKC );
		case L'D':
		case L'd':
			NEXTADVANCE( HKD );
		case L'E':
		case L'e':
			NEXTADVANCE( HKE );
		case L'L':
		case L'l':
			NEXTADVANCE( HKL );
		case L'P':
		case L'p':
			NEXTADVANCE( HKP );
		case L'U':
		case L'u':
			result = HKEY_USERS;
			NEXTADVANCE( final );
		}
	}
	STATE( HKC )
	{
		switch(*begin)
		{
		case L'C':
		case L'c':
			result = HKEY_CURRENT_CONFIG;
			NEXTADVANCE( final );
		case L'R':
		case L'r':
			result = HKEY_CLASSES_ROOT;
			NEXTADVANCE( final );
		case L'U':
		case L'u':
			result = HKEY_CURRENT_USER;
			NEXTADVANCE( final );
		default:
			return end;
		}
	}
	STATE( HKD )
	{
		switch(*begin)
		{
		case L'D':
		case L'd':
			result = HKEY_DYN_DATA;
			NEXTADVANCE( final );
		default: return end;
		}
	}
	STATE( HKE )
	{
		switch(*begin)
		{
		case L'Y':
		case L'y':
			NEXTADVANCE( HKEY );
		default: return end;
		}
	}
	STATE( HKL )
	{
		switch(*begin)
		{
		case L'M':
		case L'm':
			result = HKEY_LOCAL_MACHINE;
			NEXTADVANCE( final );
		default: return end;
		}
	}
	STATE( HKP )
	{
		switch(*begin)
		{
		case L'D':
		case L'd':
			result = HKEY_PERFORMANCE_DATA;
			NEXTADVANCE( final );
		case L'N':
		case L'n':
			result = HKEY_PERFORMANCE_NLSTEXT;
			NEXTADVANCE( final );
		case L'T':
		case L't':
			result = HKEY_PERFORMANCE_TEXT;
			NEXTADVANCE( final );
		default: return end;
		}
	}
	STATE( HKEY )
	{
		if (*begin == L'_')
			NEXTADVANCE( HKEY_ );
		return end;
	}
	STATE( HKEY_ )
	{
		switch(*begin)
		{
		case L'C':
		case L'c':
			NEXTADVANCE( HKEY_C );
		case L'D':
		case L'd':
			NEXTADVANCE( HKEY_D );
		case L'L':
		case L'l':
			NEXTADVANCE( HKEY_L );
		case L'P':
		case L'p':
			NEXTADVANCE( HKEY_P );
		case L'U':
		case L'u':
			NEXTADVANCE( HKEY_U );
		default: return end;
		}
	}
	STATE( HKEY_C )
	{
		switch(*begin)
		{
			case L'L':
			case L'l':
				NEXTADVANCE( HKEY_CL );
			case L'U':
			case L'u':
				NEXTADVANCE( HKEY_CU );
			default: return end;
		}
	}
	STATE( HKEY_CL )
	{
		switch(*begin)
		{
			case L'A':
			case L'a':
				NEXTADVANCE( HKEY_CLA );
			default: return end;
		}
	}
	STATE( HKEY_CLA )
	{
		switch(*begin)
		{
			case L'S':
			case L's':
				NEXTADVANCE( HKEY_CLAS );
			default: return end;
		}
	}
	STATE( HKEY_CLAS )
	{
		switch(*begin)
		{
			case L'S':
			case L's':
				NEXTADVANCE( HKEY_CLASS );
			default: return end;
		}
	}
	STATE( HKEY_CLASS )
	{
		switch(*begin)
		{
			case L'E':
			case L'e':
				NEXTADVANCE( HKEY_CLASSE );
			default: return end;
		}
	}
	STATE( HKEY_CLASSE )
	{
		switch(*begin)
		{
			case L'S':
			case L's':
				NEXTADVANCE( HKEY_CLASSES );
			default: return end;
		}
	}
	STATE( HKEY_CLASSES )
	{
		switch(*begin)
		{
			case L'_':
				NEXTADVANCE( HKEY_CLASSES_ );
			default: return end;
		}
	}
	STATE( HKEY_CLASSES_ )
	{
		switch(*begin)
		{
			case L'R':
			case L'r':
				NEXTADVANCE( HKEY_CLASSES_R );
			default: return end;
		}
	}
	STATE( HKEY_CLASSES_R )
	{
		switch(*begin)
		{
			case L'O':
			case L'o':
				NEXTADVANCE( HKEY_CLASSES_RO );
			default: return end;
		}
	}
	STATE( HKEY_CLASSES_RO )
	{
		switch(*begin)
		{
			case L'O':
			case L'o':
				NEXTADVANCE( HKEY_CLASSES_ROO );
			default: return end;
		}
	}
	STATE( HKEY_CLASSES_ROO )
	{
		switch(*begin)
		{
			case L'T':
			case L't':
				result = HKEY_CLASSES_ROOT;
				NEXTADVANCE( final );
			default: return end;
		}
	}
	STATE( HKEY_CU )
	{
		switch(*begin)
		{
			case L'R':
			case L'r':
				NEXTADVANCE( HKEY_CUR );
			default: return end;
		}
	}
	STATE( HKEY_CUR )
	{
		switch(*begin)
		{
			case L'R':
			case L'r':
				NEXTADVANCE( HKEY_CURR );
			default: return end;
		}
	}
	STATE( HKEY_CURR )
	{
		switch(*begin)
		{
			case L'E':
			case L'e':
				NEXTADVANCE( HKEY_CURRE );
			default: return end;
		}
	}
	STATE( HKEY_CURRE )
	{
		switch(*begin)
		{
			case L'N':
			case L'n':
				NEXTADVANCE( HKEY_CURREN );
			default: return end;
		}
	}
	STATE( HKEY_CURREN )
	{
		switch(*begin)
		{
			case L'T':
			case L't':
				NEXTADVANCE( HKEY_CURRENT );
			default: return end;
		}
	}
	STATE( HKEY_CURRENT )
	{
		switch(*begin)
		{
			case L'_':
				NEXTADVANCE( HKEY_CURRENT_ );
			default: return end;
		}
	}
	STATE( HKEY_CURRENT_ )
	{
		switch(*begin)
		{
			case L'C':
			case L'c':
				NEXTADVANCE( HKEY_CURRENT_C );
			case L'U':
			case L'u':
				NEXTADVANCE( HKEY_CURRENT_U );
			default: return end;
		}
	}
	STATE( HKEY_CURRENT_C )
	{
		switch(*begin)
		{
			case L'O':
			case L'o':
				NEXTADVANCE( HKEY_CURRENT_CO );
			default: return end;
		}
	}
	STATE( HKEY_CURRENT_CO )
	{
		switch(*begin)
		{
			case L'N':
			case L'n':
				NEXTADVANCE( HKEY_CURRENT_CON );
			default: return end;
		}
	}
	STATE( HKEY_CURRENT_CON )
	{
		switch(*begin)
		{
			case L'F':
			case L'f':
				NEXTADVANCE( HKEY_CURRENT_CONF );
			default: return end;
		}
	}
	STATE( HKEY_CURRENT_CONF )
	{
		switch(*begin)
		{
			case L'I':
			case L'i':
				NEXTADVANCE( HKEY_CURRENT_CONFI );
			default: return end;
		}
	}
	STATE( HKEY_CURRENT_CONFI )
	{
		switch(*begin)
		{
			case L'G':
			case L'g':
				result = HKEY_CURRENT_CONFIG;
				NEXTADVANCE( final );
			default: return end;
		}
	}
	STATE( HKEY_CURRENT_U )
	{
		switch(*begin)
		{
			case L'S':
			case L's':
				NEXTADVANCE( HKEY_CURRENT_US );
			default: return end;
		}
	}
	STATE( HKEY_CURRENT_US )
	{
		switch(*begin)
		{
			case L'E':
			case L'e':
				NEXTADVANCE( HKEY_CURRENT_USE );
			default: return end;
		}
	}
	STATE( HKEY_CURRENT_USE )
	{
		switch(*begin)
		{
			case L'R':
			case L'r':
				result = HKEY_CURRENT_USER;
				NEXTADVANCE( final );
			default: return end;
		}
	}
	STATE( HKEY_D )
	{
		switch(*begin)
		{
		case L'Y':
		case L'y':
			NEXTADVANCE( HKEY_DY );
		default: return end;
		}
	}
	STATE( HKEY_DY )
	{
		switch(*begin)
		{
		case L'N':
		case L'n':
			NEXTADVANCE( HKEY_DYN );
		default: return end;
		}
	}
	STATE( HKEY_DYN )
	{
		switch(*begin)
		{
		case L'_':
			NEXTADVANCE( HKEY_DYN_ );
		default: return end;
		}
	}
	STATE( HKEY_DYN_ )
	{
		switch(*begin)
		{
		case L'D':
		case L'd':
			NEXTADVANCE( HKEY_DYN_D );
		default: return end;
		}
	}
	STATE( HKEY_DYN_D )
	{
		switch(*begin)
		{
		case L'A':
		case L'a':
			NEXTADVANCE( HKEY_DYN_DA );
		default: return end;
		}
	}
	STATE( HKEY_DYN_DA )
	{
		switch(*begin)
		{
		case L'T':
		case L't':
			NEXTADVANCE( HKEY_DYN_DAT );
		default: return end;
		}
	}
	STATE( HKEY_DYN_DAT )
	{
		switch(*begin)
		{
		case L'A':
		case L'a':
			result = HKEY_DYN_DATA;
			NEXTADVANCE( final );
		default: return end;
		}
	}
	STATE( HKEY_L )
	{
		switch(*begin)
		{
		case L'O':
		case L'o':
			NEXTADVANCE( HKEY_LO );
		default: return end;
		}
	}
	STATE( HKEY_LO )
	{
		switch(*begin)
		{
		case L'C':
		case L'c':
			NEXTADVANCE( HKEY_LOC );
		default: return end;
		}
	}
	STATE( HKEY_LOC )
	{
		switch(*begin)
		{
		case L'A':
		case L'a':
			NEXTADVANCE( HKEY_LOCA );
		default: return end;
		}
	}
	STATE( HKEY_LOCA )
	{
		switch(*begin)
		{
		case L'L':
		case L'l':
			NEXTADVANCE( HKEY_LOCAL );
		default: return end;
		}
	}
	STATE( HKEY_LOCAL )
	{
		switch(*begin)
		{
		case L'_':
			NEXTADVANCE( HKEY_LOCAL_ );
		default: return end;
		}
	}
	STATE( HKEY_LOCAL_ )
	{
		switch(*begin)
		{
		case L'M':
		case L'm':
			NEXTADVANCE( HKEY_LOCAL_M );
		default: return end;
		}
	}
	STATE( HKEY_LOCAL_M )
	{
		switch(*begin)
		{
		case L'A':
		case L'a':
			NEXTADVANCE( HKEY_LOCAL_MA );
		default: return end;
		}
	}
	STATE( HKEY_LOCAL_MA )
	{
		switch(*begin)
		{
		case L'C':
		case L'c':
			NEXTADVANCE( HKEY_LOCAL_MAC );
		default: return end;
		}
	}
	STATE( HKEY_LOCAL_MAC )
	{
		switch(*begin)
		{
		case L'H':
		case L'h':
			NEXTADVANCE( HKEY_LOCAL_MACH );
		default: return end;
		}
	}
	STATE( HKEY_LOCAL_MACH )
	{
		switch(*begin)
		{
		case L'I':
		case L'i':
			NEXTADVANCE( HKEY_LOCAL_MACHI );
		default: return end;
		}
	}
	STATE( HKEY_LOCAL_MACHI )
	{
		switch(*begin)
		{
		case L'N':
		case L'n':
			NEXTADVANCE( HKEY_LOCAL_MACHIN );
		default: return end;
		}
	}
	STATE( HKEY_LOCAL_MACHIN )
	{
		switch(*begin)
		{
		case L'E':
		case L'e':
			result = HKEY_LOCAL_MACHINE;
			NEXTADVANCE( final );
		default: return end;
		}
	}
	STATE( HKEY_P )
	{
		switch(*begin)
		{
		case L'E':
		case L'e':
			NEXTADVANCE( HKEY_PE );
		default: return end;
		}
	}
	STATE( HKEY_PE )
	{
		switch(*begin)
		{
		case L'R':
		case L'r':
			NEXTADVANCE( HKEY_PER );
		default: return end;
		}
	}
	STATE( HKEY_PER )
	{
		switch(*begin)
		{
		case L'F':
		case L'f':
			NEXTADVANCE( HKEY_PERF );
		default: return end;
		}
	}
	STATE( HKEY_PERF )
	{
		switch(*begin)
		{
		case L'O':
		case L'o':
			NEXTADVANCE( HKEY_PERFO );
		default: return end;
		}
	}
	STATE( HKEY_PERFO )
	{
		switch(*begin)
		{
		case L'R':
		case L'r':
			NEXTADVANCE( HKEY_PERFOR );
		default: return end;
		}
	}
	STATE( HKEY_PERFOR )
	{
		switch(*begin)
		{
		case L'M':
		case L'm':
			NEXTADVANCE( HKEY_PERFORM );
		default: return end;
		}
	}
	STATE( HKEY_PERFORM )
	{
		switch(*begin)
		{
		case L'A':
		case L'a':
			NEXTADVANCE( HKEY_PERFORMA );
		default: return end;
		}
	}
	STATE( HKEY_PERFORMA )
	{
		switch(*begin)
		{
		case L'N':
		case L'n':
			NEXTADVANCE( HKEY_PERFORMAN );
		default: return end;
		}
	}
	STATE( HKEY_PERFORMAN )
	{
		switch(*begin)
		{
		case L'C':
		case L'c':
			NEXTADVANCE( HKEY_PERFORMANC );
		default: return end;
		}
	}
	STATE( HKEY_PERFORMANC )
	{
		switch(*begin)
		{
		case L'E':
		case L'e':
			NEXTADVANCE( HKEY_PERFORMANCE );
		default: return end;
		}
	}
	STATE( HKEY_PERFORMANCE )
	{
		switch(*begin)
		{
		case L'_':
			NEXTADVANCE( HKEY_PERFORMANCE_ );
		default: return end;
		}
	}
	STATE( HKEY_PERFORMANCE_ )
	{
		switch(*begin)
		{
		case L'D':
		case L'd':
			NEXTADVANCE( HKEY_PERFORMANCE_D );
		case L'N':
		case L'n':
			NEXTADVANCE( HKEY_PERFORMANCE_N );
		case L'T':
		case L't':
			NEXTADVANCE( HKEY_PERFORMANCE_T );
		default: return end;
		}
	}
	STATE( HKEY_PERFORMANCE_D )
	{
		switch(*begin)
		{
		case L'A':
		case L'a':
			NEXTADVANCE( HKEY_PERFORMANCE_DA );
		default: return end;
		}
	}
	STATE( HKEY_PERFORMANCE_DA )
	{
		switch(*begin)
		{
		case L'T':
		case L't':
			NEXTADVANCE( HKEY_PERFORMANCE_DAT );
		default: return end;
		}
	}
	STATE( HKEY_PERFORMANCE_DAT )
	{
		switch(*begin)
		{
		case L'A':
		case L'a':
			result = HKEY_PERFORMANCE_DATA;
			NEXTADVANCE( final );
		default: return end;
		}
	}
	STATE( HKEY_PERFORMANCE_N )
	{
		switch(*begin)
		{
		case L'L':
		case L'l':
			NEXTADVANCE( HKEY_PERFORMANCE_NL );
		default: return end;
		}
	}
	STATE( HKEY_PERFORMANCE_NL )
	{
		switch(*begin)
		{
		case L'S':
		case L's':
			NEXTADVANCE( HKEY_PERFORMANCE_NLS );
		default: return end;
		}
	}
	STATE( HKEY_PERFORMANCE_NLS )
	{
		switch(*begin)
		{
		case L'T':
		case L't':
			NEXTADVANCE( HKEY_PERFORMANCE_NLST );
		default: return end;
		}
	}
	STATE( HKEY_PERFORMANCE_NLST )
	{
		switch(*begin)
		{
		case L'E':
		case L'e':
			NEXTADVANCE( HKEY_PERFORMANCE_NLSTE );
		default: return end;
		}
	}
	STATE( HKEY_PERFORMANCE_NLSTE )
	{
		switch(*begin)
		{
		case L'X':
		case L'x':
			NEXTADVANCE( HKEY_PERFORMANCE_NLSTEX );
		default: return end;
		}
	}
	STATE( HKEY_PERFORMANCE_NLSTEX )
	{
		switch(*begin)
		{
		case L'T':
		case L't':
			result = HKEY_PERFORMANCE_NLSTEXT;
			NEXTADVANCE( final );
		default: return end;
		}
	}
	STATE( HKEY_PERFORMANCE_T )
	{
		switch(*begin)
		{
		case L'E':
		case L'e':
			NEXTADVANCE( HKEY_PERFORMANCE_TE );
		default: return end;
		}
	}
	STATE( HKEY_PERFORMANCE_TE )
	{
		switch(*begin)
		{
		case L'X':
		case L'x':
			NEXTADVANCE( HKEY_PERFORMANCE_TEX );
		default: return end;
		}
	}
	STATE( HKEY_PERFORMANCE_TEX )
	{
		switch(*begin)
		{
		case L'T':
		case L't':
			result = HKEY_PERFORMANCE_TEXT;
			NEXTADVANCE( final );
		default: return end;
		}
	}
	STATE( HKEY_U )
	{
		switch(*begin)
		{
		case L'S':
		case L's':
			NEXTADVANCE( HKEY_US );
		default: return end;
		}
	}
	STATE( HKEY_US )
	{
		switch(*begin)
		{
		case L'E':
		case L'e':
			NEXTADVANCE( HKEY_USE );
		default: return end;
		}
	}
	STATE( HKEY_USE )
	{
		switch(*begin)
		{
		case L'R':
		case L'r':
			NEXTADVANCE( HKEY_USER );
		default: return end;
		}
	}
	STATE( HKEY_USER )
	{
		switch(*begin)
		{
		case L'S':
		case L's':
			result = HKEY_USERS;
			NEXTADVANCE( final );
		default: return end;
		}
	}
	STATE( final )
	{
		if (*begin == L'\\')
		{
			begin++;
			if(begin == end)
				return end;
		}
		if (*begin == L']')
			begin++;
		return begin;
	}
	return end;
}
}
