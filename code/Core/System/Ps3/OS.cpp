#include "Core/System/OS.h"
#include "Core/Singleton/SingletonManager.h"

namespace traktor
{

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
	return 2;
}

std::wstring OS::getComputerName() const
{
	return L"PS3";
}

std::wstring OS::getCurrentUser() const
{
	return L"User";
}

std::wstring OS::getUserHomePath() const
{
	return L".";
}

std::wstring OS::getUserApplicationDataPath() const
{
	return L".";
}

std::wstring OS::getWritableFolderPath() const
{
	return L".";
}

bool OS::editFile(const Path& file) const
{
	return false;
}

bool OS::exploreFile(const Path& file) const
{
	return false;
}

Ref< IProcess > OS::execute(const Path& file, const std::wstring& commandLine, const Path& workingDirectory, bool mute) const
{
	return 0;
}

Ref< ISharedMemory > OS::createSharedMemory(const std::wstring& name, uint32_t size) const
{
	return 0;
}

OS::OS()
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
