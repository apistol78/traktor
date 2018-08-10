/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Windows.h>
#include <detours.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/Path.h>
#include <Core/Log/Log.h>
#include <Core/Misc/CommandLine.h>
#include <Core/System/OS.h>

using namespace traktor;

// {0C8ACD8F-9C7E-4076-B859-CFC0CA4C548C}
static const GUID c_guidShadowPayload = { 0xc8acd8f, 0x9c7e, 0x4076, { 0xb8, 0x59, 0xcf, 0xc0, 0xca, 0x4c, 0x54, 0x8c } };

#pragma pack(1)
struct ShadowPayload
{
	wchar_t sourcePath[MAX_PATH];
	wchar_t sandboxPath[MAX_PATH];
};
#pragma pack()

static BOOL CALLBACK ListFileCallback(PVOID pContext, LPCSTR pszOrigFile, LPCSTR pszFile, LPCSTR *ppszOutFile)
{  
	std::set< std::wstring >& dlls = *(std::set< std::wstring >*)pContext;
	dlls.insert(mbstows(pszFile));
	return TRUE;   
}   

bool shadowCopy(const Path& shadowPath, const Path& sourceFile)
{
	Path shadowFile = shadowPath + sourceFile.getFileName();

	if (FileSystem::getInstance().get(shadowFile) != 0)
		return true;

	FileSystem::getInstance().makeAllDirectories(shadowPath);

	HANDLE hSourceFile = CreateFile(
		sourceFile.getPathName().c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hSourceFile == INVALID_HANDLE_VALUE)
		return false;

	HANDLE hShadowFile = CreateFile(
		shadowFile.getPathName().c_str(),
		GENERIC_WRITE | GENERIC_READ,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
		NULL
	);
	if (hShadowFile == INVALID_HANDLE_VALUE)
		return false;

	PDETOUR_BINARY pBinary = DetourBinaryOpen(hSourceFile);
	if (!pBinary)
	{
		CloseHandle(hSourceFile);
		return false;
	}

	// Scan all import DLLs from binary.
	std::set< std::wstring > dlls;
	DetourBinaryEditImports(pBinary, &dlls, NULL, ListFileCallback, NULL, NULL);

	// Append our payload to binary.
	ShadowPayload pl = { 0 };
	wcscpy(pl.sourcePath, sourceFile.getPathOnly().c_str());
	wcscpy(pl.sandboxPath, shadowPath.getPathName().c_str());
	DetourBinarySetPayload(pBinary, c_guidShadowPayload, &pl, sizeof(pl));

	// Write our patch binary.
	DetourBinaryWrite(pBinary, hShadowFile);

	DetourBinaryClose(pBinary);

	CloseHandle(hShadowFile);
	CloseHandle(hSourceFile);

	for (std::set< std::wstring >::const_iterator i = dlls.begin(); i != dlls.end(); ++i)
	{
		if (FileSystem::getInstance().get(sourceFile.getPathOnly() + L"/" + *i) != 0)
			shadowCopy(shadowPath, sourceFile.getPathOnly() + L"/" + *i);
	}

	return true;
}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);
	if (cmdLine.getCount() < 2)
	{
		log::info << L"Usable: ShadowLaunch [sandbox] [executable] (command line)" << Endl;
		return 1;
	}

	std::wstring sandbox = cmdLine.getString(0);
	Path executableFile = cmdLine.getString(1);

	Path shadowPath = OS::getInstance().getWritableFolderPath() + L"/Doctor Entertainment AB/Sandbox/" + sandbox;
	Path shadowFile = shadowPath + executableFile.getFileName();

	RefArray< File > files;
	FileSystem::getInstance().find(shadowPath + L"*.*", files);
	for (RefArray< File >::const_iterator i = files.begin(); i != files.end(); ++i)
		FileSystem::getInstance().remove((*i)->getPath());

	if (!shadowCopy(shadowPath, executableFile))
	{
		log::error << L"Unable to sandbox file \"" << executableFile.getPathName() << L"\"." << Endl;
		return 1;
	}

	WCHAR fn[MAX_PATH];
	GetModuleFileName(NULL, fn, MAX_PATH);

	Path hookDll = Path(fn).getPathOnly() + L"/ShadowLaunchHook.dll";

	CHAR hd[MAX_PATH] = { 0 };
	strcpy(hd, wstombs(hookDll.getPathName()).c_str());

	WCHAR cl[MAX_PATH] = { 0 };
	wcscpy(cl, shadowFile.getPathName().c_str());

	if (cmdLine.getCount() >= 3)
	{
		wcscat(cl, L" ");
		wcscat(cl, cmdLine.getString(2).c_str());
	}

	WCHAR cwd[MAX_PATH] = { 0 };
	Path currentDir = FileSystem::getInstance().getCurrentVolumeAndDirectory();
	wcscpy(cwd, currentDir.getPathName().c_str());

	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };

	DetourCreateProcessWithDll(
		NULL,
		cl,
		NULL,
		NULL,
		FALSE,
		CREATE_DEFAULT_ERROR_MODE,
		NULL,
		cwd,
		&si,
		&pi,
		hd,
		NULL
	);

	return 0;
}
