#include <unistd.h>
#include <sys/param.h>
#include <pwd.h>
#include "Core/System/OS.h"
#include "Core/System/Linux/SharedMemoryLinux.h"
#include "Core/Heap/Heap.h"
#include "Core/Heap/GcNew.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Misc/TString.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.OS", OS, Singleton)

OS& OS::getInstance()
{
	static OS* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new OS();
		SingletonManager::getInstance().addBefore(s_instance, &Heap::getInstance());
	}
	return *s_instance;
}

uint32_t OS::getCPUCoreCount() const
{
	return 4;
}

std::wstring OS::getComputerName() const
{
	char name[MAXHOSTNAMELEN];

	if (gethostname(name, sizeof_array(name)) != -1)
		return mbstows(name);

	return L"Unavailable";
}

std::wstring OS::getCurrentUser() const
{
	passwd* pwd = getpwuid(geteuid());
	if (!pwd)
		return L"Unavailable";

	const char* who = pwd->pw_name;
	if (!who)
		return L"Unavailable";

	return mbstows(who);
}

std::wstring OS::getUserHomePath() const
{
	return L"~";
}

std::wstring OS::getUserApplicationDataPath() const
{
	return L"~/.AppData";
}

std::wstring OS::getWritableFolderPath() const
{
	return L"~/.AppData/.Temp";
}

bool OS::editFile(const Path& file) const
{
	return false;
}

bool OS::exploreFile(const Path& file) const
{
	return false;
}

Process* OS::execute(const Path& file, const std::wstring& commandLine, const Path& workingDirectory, bool mute) const
{
	return 0;
}

SharedMemory* OS::createSharedMemory(const std::wstring& name, uint32_t size) const
{
	return gc_new< SharedMemoryLinux >(size);
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
