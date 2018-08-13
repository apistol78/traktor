/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Windows.h>
#include <detours.h>
#include "Core/Guid.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Path.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/System/OS.h"

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

bool shadowCopy(const Path& sandboxPath, const Path& sourceFile)
{
	Path shadowFile = sandboxPath + sourceFile.getFileName();

	if (FileSystem::getInstance().get(shadowFile) != 0)
		return true;

	FileSystem::getInstance().makeAllDirectories(sandboxPath);

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
	wcscpy(pl.sandboxPath, sandboxPath.getPathName().c_str());
	DetourBinarySetPayload(pBinary, c_guidShadowPayload, &pl, sizeof(pl));

	// Write our patch binary.
	DetourBinaryWrite(pBinary, hShadowFile);

	DetourBinaryClose(pBinary);

	CloseHandle(hShadowFile);
	CloseHandle(hSourceFile);

	for (std::set< std::wstring >::const_iterator i = dlls.begin(); i != dlls.end(); ++i)
	{
		if (FileSystem::getInstance().get(sourceFile.getPathOnly() + L"/" + *i) != 0)
			shadowCopy(sandboxPath, sourceFile.getPathOnly() + L"/" + *i);
	}

	return true;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR szCmdLine, int)
{
	wchar_t file[MAX_PATH] = L"";
	GetModuleFileName(NULL, file, sizeof_array(file));

	CommandLine cmdLine(file, mbstows(szCmdLine));
	if (cmdLine.getCount() < 1)
	{
		log::info << L"Traktor.ShadowLaunch.App; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;
		log::info << Endl;
		log::info << L"Usable: Traktor.ShadowLaunch.App [executable] (command line)" << Endl;
		return 1;
	}

	Path executableFile = cmdLine.getString(0);
	std::wstring sandbox = Guid::create().format();

	// Find old sandbox matching same command line.
	RefArray< File > sandboxes;
	FileSystem::getInstance().find(OS::getInstance().getWritableFolderPath() + L"/Doctor Entertainment AB/Sandbox/*.*", sandboxes);
	for (RefArray< File >::const_iterator i = sandboxes.begin(); i != sandboxes.end(); ++i)
	{
		const Path& p = (*i)->getPath();
		if (p.getFileName() == L"." || p.getFileName() == L"..")
			continue;

		Ref< traktor::IStream > f = FileSystem::getInstance().open(p + L".commandLine", File::FmRead);
		if (!f)
			continue;

		std::wstring commandLine;
		if (StringReader(f, &Utf8Encoding()).readLine(commandLine) <= 0)
			continue;

		f->close();
		f = nullptr;

		if (commandLine == mbstows(szCmdLine))
		{
			if (FileSystem::getInstance().remove(p + L".commandLine"))
			{
				sandbox = p.getFileName();
				break;
			}
		}
	}

	Path shadowPath = OS::getInstance().getWritableFolderPath() + L"/Doctor Entertainment AB/Sandbox/" + sandbox;
	Path shadowFile = shadowPath + executableFile.getFileName();

	if (!shadowCopy(shadowPath, executableFile))
	{
		log::error << L"Unable to sandbox file \"" << executableFile.getPathName() << L"\"." << Endl;
		return 1;
	}

	WCHAR fn[MAX_PATH];
	GetModuleFileName(NULL, fn, MAX_PATH);

	Path hookDll = Path(fn).getPathOnly() + L"/Traktor.ShadowLaunch.Hook.dll";

	CHAR hd[MAX_PATH] = { 0 };
	strcpy(hd, wstombs(hookDll.getPathName()).c_str());

	WCHAR cl[MAX_PATH] = { 0 };
	wcscpy(cl, shadowFile.getPathName().c_str());

	for (size_t i = 1; i < cmdLine.getCount(); ++i)
	{
		wcscat(cl, L" ");
		wcscat(cl, cmdLine.getString(i).c_str());
	}

	WCHAR cwd[MAX_PATH] = { 0 };
	Path currentDir = FileSystem::getInstance().getCurrentVolumeAndDirectory();
	wcscpy(cwd, currentDir.getPathName().c_str());

	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };

	BOOL result = DetourCreateProcessWithDll(
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
	if (result == FALSE)
	{
		log::error << L"Unable to spawn sandboxed process \"" << shadowFile.getPathName() << L"\"." << Endl;
		return 1;
	}

	if (WaitForSingleObject(pi.hProcess, INFINITE) != WAIT_OBJECT_0)
	{
		log::error << L"Failed to wait for sandboxed process, unable to cleanup sandbox." << Endl;
		return 1;
	}

	// Cleanup sandbox.
	RefArray< File > files;
	FileSystem::getInstance().find(shadowPath + L"*.*", files);
	for (RefArray< File >::const_iterator i = files.begin(); i != files.end(); ++i)
		FileSystem::getInstance().remove((*i)->getPath());

	// Save command line in sandbox so it can be reused.
	Ref< traktor::IStream > f = FileSystem::getInstance().open(shadowPath + L".commandLine", File::FmWrite);
	if (f)
	{
		FileOutputStream(f, &Utf8Encoding()) << mbstows(szCmdLine) << Endl;
		f->close();
	}

	// Return using same exit code as child process.
	DWORD code = 0;
	GetExitCodeProcess(pi.hProcess, &code);
	return (int)code;
}
