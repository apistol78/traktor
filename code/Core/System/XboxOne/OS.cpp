/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
//#define _WIN32_LEAN_AND_MEAN
//#include <windows.h>
//#include <lmcons.h>
//#include <shlobj.h>
//#include <tchar.h>
//#include "Core/Io/IVolume.h"
//#include "Core/Io/FileSystem.h"
//#include "Core/Io/StringOutputStream.h"
//#include "Core/Log/Log.h"
//#include "Core/Misc/AutoPtr.h"
//#include "Core/Misc/String.h"
//#include "Core/Misc/TString.h"
#include "Core/Singleton/SingletonManager.h"
//#include "Core/System/Environment.h"
#include "Core/System/OS.h"
//#include "Core/System/ResolveEnv.h"
//#include "Core/System/Win32/ProcessWin32.h"
//#include "Core/System/Win32/SharedMemoryWin32.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.OS", OS, Object)

OS& OS::getInstance()
{
	static OS* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new OS();
		s_instance->addRef(0);
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

uint32_t OS::getCPUCoreCount() const
{
	return 1;
}

Path OS::getExecutable() const
{
	return L"";
}

std::wstring OS::getCommandLine() const
{
	return L"";
}

std::wstring OS::getComputerName() const
{
	return L"";
}

std::wstring OS::getCurrentUser() const
{
	return L"";
}

std::wstring OS::getUserHomePath() const
{
	return L"";
}

std::wstring OS::getUserApplicationDataPath() const
{
	return L"";
}

std::wstring OS::getWritableFolderPath() const
{
	return L"";
}

bool OS::openFile(const std::wstring& file) const
{
	return false;
}

bool OS::editFile(const std::wstring& file) const
{
	return false;
}

bool OS::exploreFile(const std::wstring& file) const
{
	return false;
}

bool OS::setEnvironment(const std::wstring& name, const std::wstring& value) const
{
	return false;
}

Ref< Environment > OS::getEnvironment() const
{
	return 0;
}

bool OS::getEnvironment(const std::wstring& name, std::wstring& outValue) const
{
	return false;
}

Ref< IProcess > OS::execute(
	const std::wstring& commandLine,
	const Path& workingDirectory,
	const Environment* env,
	bool redirect,
	bool mute,
	bool detach
) const
{
	return 0;
}

Ref< ISharedMemory > OS::createSharedMemory(const std::wstring& name, uint32_t size) const
{
	return 0;
}

bool OS::setOwnProcessPriorityBias(int32_t priorityBias)
{
	return false;
}

bool OS::getRegistry(const std::wstring& key, const std::wstring& subKey, const std::wstring& valueName, std::wstring& outValue) const
{
	return false;
}

OS::OS()
{
}

OS::~OS()
{
}

void OS::destroy()
{
	T_SAFE_RELEASE(this);
}

}
