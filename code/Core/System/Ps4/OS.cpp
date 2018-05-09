/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/System/Environment.h"
#include "Core/System/OS.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.OS", OS, Object)

OS& OS::getInstance()
{
	static OS* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new OS();
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

uint32_t OS::getCPUCoreCount() const
{
	return 6;
}

Path OS::getExecutable() const
{
	return Path();
}

std::wstring OS::getCommandLine() const
{
	return L"";
}

std::wstring OS::getComputerName() const
{
	return L"PS4";
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
	return new Environment();
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

OS::OS()
:	m_handle(0)
{
}

OS::~OS()
{
}

void OS::destroy()
{
	delete this;
}

}
