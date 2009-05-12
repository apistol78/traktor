#include <unistd.h>
#include <sys/param.h>
#include <pwd.h>
#include "Core/System/OS.h"
#include "Core/Heap/Heap.h"
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

bool OS::editFile(const Path& file) const
{
	return false;
}

bool OS::exploreFile(const Path& file) const
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
	delete this;
}

}
