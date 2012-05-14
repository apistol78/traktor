#include <cstring>
#include <spawn.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include "Core/Io/Path.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Misc/TString.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/System/OS.h"
#include "Core/System/Linux/ProcessLinux.h"
#include "Core/System/Linux/SharedMemoryLinux.h"

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

OS::envmap_t OS::getEnvironment() const
{
	return envmap_t();
}

bool OS::getEnvironment(const std::wstring& name, std::wstring& outValue) const
{
    return false;
}

Ref< IProcess > OS::execute(
	const Path& file,
	const std::wstring& commandLine,
	const Path& workingDirectory,
	const envmap_t* envmap,
	bool redirect,
	bool mute,
	bool detach
) const
{
	char cwd[512];
	char* envv[256];
	char* argv[64];
	int envc = 0;
	int argc = 0;
	int err;

	std::string fileName = wstombs(file.getPathNameNoVolume());

	// Convert all arguments; append bash if executing shell script.
	if (compareIgnoreCase< std::wstring >(file.getExtension(), L"sh") == 0)
		argv[argc++] = strdup("/bin/bash");
	else
	{
		// Ensure file has executable permission.
		struct stat st;
		if (stat(fileName.c_str(), &st) == 0)
		{
			if (chmod(fileName.c_str(), st.st_mode | S_IXUSR) != 0)
				log::warning << L"Unable to set mode of file \"" << file.getPathName() << L"\"" << Endl;
		}
		else
			log::warning << L"Unable to get mode of file \"" << file.getPathName() << L"\"" << Endl;
	}

	argv[argc++] = strdup(fileName.c_str());

	StringSplit< std::wstring > s(commandLine, L" ");
	for (StringSplit< std::wstring >::const_iterator i = s.begin(); i != s.end(); ++i)
		argv[argc++] = strdup(wstombs(*i).c_str());

	// Convert environment variables.
	if (envmap)
	{
		for (envmap_t::const_iterator i = envmap->begin(); i != envmap->end(); ++i)
			envv[envc++] = strdup(wstombs(i->first + L"=" + i->second).c_str());
	}

	// Terminate argument and environment vectors.
	envv[envc] = 0;
	argv[argc] = 0;

	// Spawned process inherit working directory from our process; thus
	// we need to temporarily change directory.
	getcwd(cwd, sizeof(cwd));
	chdir(wstombs(workingDirectory.getPathNameNoVolume()).c_str());

	// Redirect standard IO.
	/*
	posix_spawn_file_actions_t fileActions;
	posix_spawn_file_actions_init(&fileActions);
	posix_spawn_file_actions_adddup2(&fileActions, 1, 2);
	*/

	// Spawn process.
	pid_t pid;
	err = posix_spawn(&pid, argv[0], 0, 0, argv, envv);

	// Restore our working directory before returning.
	chdir(cwd);

	if (err != 0)
		return 0;

	return new ProcessLinux(pid);
}

Ref< ISharedMemory > OS::createSharedMemory(const std::wstring& name, uint32_t size) const
{
	return new SharedMemoryLinux(size);
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
