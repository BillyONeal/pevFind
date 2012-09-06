//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// linkResolve.h -- Implements the linkResolve subprogram and library function.

#include "pch.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Objbase.h>
#include <ObjIdl.h>
#include <shobjidl.h>
#include "linkResolve.h"
#include "utility.h"
#include "logger.h"

namespace linkResolve
{

struct comSetup
{
	comSetup()
	{
		CoInitialize(NULL);
	};
	~comSetup()
	{
		CoUninitialize();
	};
};

int main(int argc, wchar_t * argv[])
{	
	comSetup refrenceManager;
	UNREFERENCED_PARAMETER(refrenceManager);
	if (argc > 2)
	{
		for (int cur = 1; cur != argc; cur++)
		{
			std::wstring currentString(argv[cur]);
			logger << currentString << L" -> " << resolveLink(currentString) << L"\r\n";
		}
	} else
	{
		std::wstring currentString(argv[1]);
		logger << resolveLink(currentString) << L"\r\n";
	}
	return 0;
}

std::wstring resolveLink(std::wstring& lnkPath)
{
	IPersistFile * persistFile;
	IShellLink * theLink;
	HRESULT errorCheck = 0;
	wchar_t linkTarget[MAX_PATH];
	wchar_t expandedTarget[MAX_PATH];
	wchar_t arguments[INFOTIPSIZE];
	errorCheck = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IPersistFile, reinterpret_cast<void **>(&persistFile));
	if (!SUCCEEDED(errorCheck)) goto error;
	if (!SUCCEEDED(persistFile->Load(lnkPath.c_str(), 0))) goto unloadPersist;
	errorCheck = persistFile->QueryInterface(IID_IShellLinkW, reinterpret_cast<void **>(&theLink));
	if (!SUCCEEDED(errorCheck)) goto unloadPersist;
	if (!SUCCEEDED(theLink->Resolve(NULL, SLR_NO_UI))) goto unloadLink;
	if (!SUCCEEDED(theLink->GetPath(linkTarget, MAX_PATH, NULL, SLGP_RAWPATH))) goto unloadLink;
	errorCheck = theLink->GetArguments(arguments, INFOTIPSIZE);
	if (!SUCCEEDED(errorCheck)) arguments[0] = L'\0';
	ExpandEnvironmentStrings(linkTarget, expandedTarget, MAX_PATH);
	persistFile->Release();
	theLink->Release();
	return std::wstring(expandedTarget) + L" " + arguments;

unloadLink:
	theLink->Release();
unloadPersist:
	persistFile->Release();
error:
	return L"";
}

} //namespace linkResolve
