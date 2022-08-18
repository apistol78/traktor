#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <limits.h>
#include <spawn.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include "Core/Io/FileSystem.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Misc/TString.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/System/Environment.h"
#include "Core/System/ResolveEnv.h"
#include "Core/System/Linux/ProcessLinux.h"
#include "Core/System/Linux/SharedMemoryLinux.h"

namespace traktor
{
	namespace
	{

void handle_sigchld(int sig) 
{
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.OS", OS, Object)

OS& OS::getInstance()
{
	static OS* s_instance = nullptr;
	if (!s_instance)
	{
		s_instance = new OS();
		s_instance->addRef(0);
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

std::wstring OS::getName() const
{
	return L"Linux";	
}

std::wstring OS::getIdentifier() const
{
	return L"linux";
}

uint32_t OS::getCPUCoreCount() const
{
	return (uint32_t)sysconf(_SC_NPROCESSORS_ONLN);
}

Path OS::getExecutable() const
{
	char result[PATH_MAX] = { 0 };
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	if (count > 0)
		return Path(mbstows(result));
	else
		return Path();
}

std::wstring OS::getCommandLine() const
{
	char cmdLine[1024] = { '\0' };
	FILE* fp = fopen("/proc/self/cmdline", "r");
	if (fp)
	{
		if (fgets(cmdLine, sizeof(cmdLine), fp) == nullptr)
			cmdLine[0] = '\0';
		fclose(fp);
	}
	return mbstows(cmdLine);
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
	std::wstring home;
	if (getEnvironment(L"HOME", home))
		return home;
	else
		return L"~";
}

std::wstring OS::getUserApplicationDataPath() const
{
	return getUserHomePath() + L"/.traktor";
}

std::wstring OS::getWritableFolderPath() const
{
	return getUserHomePath() + L"/.traktor";
}

bool OS::openFile(const std::wstring& file) const
{
	return system(("xdg-open " + wstombs(file)).c_str()) != 0;
}

bool OS::editFile(const std::wstring& file) const
{
	return system(("xdg-open " + wstombs(file)).c_str()) != 0;
}

bool OS::exploreFile(const std::wstring& file) const
{
	return false;
}

bool OS::setEnvironment(const std::wstring& name, const std::wstring& value) const
{
	return bool(setenv(
		wstombs(name).c_str(),
		wstombs(value).c_str(),
		1
	) == 0);
}

Ref< Environment > OS::getEnvironment() const
{
	Ref< Environment > env = new Environment();
	for (char** e = environ; *e; ++e)
	{
		char* sep = strchr(*e, '=');
		if (sep)
		{
			std::wstring ek = mbstows(std::string(*e, sep));
			if (!ek.empty())
			{
				char* val = sep + 1;
				env->set(ek, mbstows(val));
			}
		}
	}
	return env;
}

bool OS::getEnvironment(const std::wstring& name, std::wstring& outValue) const
{
	const char* value = getenv(wstombs(name).c_str());
	if (value)
	{
		outValue = mbstows(value);
		return true;
	}
	else
		return false;
}

Ref< IProcess > OS::execute(
	const std::wstring& commandLine,
	const Path& workingDirectory,
	const Environment* env,
	uint32_t flags
) const
{
	posix_spawn_file_actions_t* fileActions = nullptr;
	posix_spawnattr_t* spawnAttrp = nullptr;
	AlignedVector< char* > envv;
	AlignedVector< char* > argv;
	char cwd[4096];
	int envc = 0;
	int argc = 0;
	int err;
	pid_t pid;
	int childStdOut[2] = { 0, 0 };
	int childStdErr[2] = { 0, 0 };

	// Resolve entire command line.
	std::wstring resolvedCommandLine = resolveEnv(commandLine, env);

	// Split command line into argv.
	AlignedVector< std::wstring > resolvedArguments;
	if (!splitCommandLine(resolvedCommandLine, resolvedArguments))
		return nullptr;
	if (resolvedArguments.empty())
		return nullptr;

	// Extract executable file.
	std::wstring executable = resolvedArguments.front();
	if (executable.empty())
		return nullptr;

	// Since Raspberry PI doesn't support changing working directory
	// in posix spawn we need to launch through "env" shim.
#if defined(__RPI__)
	if (!workingDirectory.empty())
	{
		Path awd = FileSystem::getInstance().getAbsolutePath(workingDirectory);
		strcpy(cwd, wstombs(awd.getPathNameNoVolume()).c_str());

		argv.push_back(strdup("/bin/env"));
		argv.push_back(strdup("-C"));
		argv.push_back(strdup(cwd));
	}
#else
	Path awd = FileSystem::getInstance().getAbsolutePath(workingDirectory);
	if (awd.empty())
		return nullptr;
	strcpy(cwd, wstombs(awd.getPathNameNoVolume()).c_str());
#endif

	// Start with bash if executing shell script.
	if (endsWith(executable, L".sh"))
		argv.push_back(strdup("/bin/sh"));

	// Convert all arguments into utf-8.
	argv.push_back(strdup(wstombs(executable).c_str()));
	for (auto it = resolvedArguments.begin() + 1; it != resolvedArguments.end(); ++it)
		argv.push_back(strdup(wstombs(*it).c_str()));

	// Convert environment variables.
	if (env)
	{
		for (auto it : env->get())
			envv.push_back(strdup(wstombs(it.first + L"=" + it.second).c_str()));
	}

	// Terminate argument and environment vectors.
	envv.push_back(nullptr);
	argv.push_back(nullptr);

	// Redirect standard IO.
	if ((flags & EfRedirectStdIO) != 0)
	{
		if (pipe(childStdOut) != 0)
			return nullptr;
		if (pipe(childStdErr) != 0)
			return nullptr;

		spawnAttrp = new posix_spawnattr_t;
		posix_spawnattr_init(spawnAttrp);
		posix_spawnattr_setflags(spawnAttrp, POSIX_SPAWN_SETPGROUP);

		fileActions = new posix_spawn_file_actions_t;
		posix_spawn_file_actions_init(fileActions);
#if !defined(__RPI__)
		posix_spawn_file_actions_addchdir_np(fileActions, cwd);
#endif
		posix_spawn_file_actions_addclose(fileActions, childStdOut[0]);
		posix_spawn_file_actions_addclose(fileActions, childStdErr[0]);
		posix_spawn_file_actions_adddup2(fileActions, childStdOut[1], STDOUT_FILENO);
		posix_spawn_file_actions_adddup2(fileActions, childStdErr[1], STDERR_FILENO);
		posix_spawn_file_actions_addclose(fileActions, childStdOut[1]);
		posix_spawn_file_actions_addclose(fileActions, childStdErr[1]);

		// Spawn process.
		err = posix_spawnp(&pid, argv[0], fileActions, 0, argv.ptr(), env ? envv.ptr() : environ);
	}
	else
	{
		spawnAttrp = new posix_spawnattr_t;
		posix_spawnattr_init(spawnAttrp);
		posix_spawnattr_setflags(spawnAttrp, POSIX_SPAWN_SETPGROUP);

		fileActions = new posix_spawn_file_actions_t;
		posix_spawn_file_actions_init(fileActions);
#if !defined(__RPI__)
		posix_spawn_file_actions_addchdir_np(fileActions, cwd);
#endif
		// Spawn process.
		err = posix_spawnp(&pid, argv[0], fileActions, 0, argv.ptr(), env ? envv.ptr() : environ);
	}

	// Free arguments.
	for (auto arg : argv)
	{
		if (arg)
			free(arg);
	}
	for (auto env : envv)
	{
		if (env)
			free(env);
	}

	// Cleanup in case of failure.
	if (err != 0)
	{
		posix_spawn_file_actions_destroy(fileActions);
		delete fileActions;

		posix_spawnattr_destroy(spawnAttrp);
		delete spawnAttrp;

		return nullptr;
	}

	return new ProcessLinux(
		pid, 
		fileActions,
		spawnAttrp,
		childStdOut[0],
		childStdErr[0]
	);
}

Ref< ISharedMemory > OS::createSharedMemory(const std::wstring& name, uint32_t size) const
{
	Ref< SharedMemoryLinux > shm = new SharedMemoryLinux();
	if (shm->create(name, size))
		return shm;
	else
		return nullptr;
}

bool OS::setOwnProcessPriorityBias(int32_t priorityBias)
{
	return false;
}

bool OS::whereIs(const std::wstring& executable, Path& outPath) const
{
	std::wstring paths;

	// Get system "PATH" environment variable.
	if (!getEnvironment(L"PATH", paths))
		return false;

	// Try to locate binary in any of the paths specified in "PATH".
	for (auto path : StringSplit< std::wstring >(paths, L";:,"))
	{
		Ref< File > file = FileSystem::getInstance().get(path + L"/" + executable);
		if (file)
		{
			outPath = file->getPath();
			return true;
		}
	}

	return false;
}

OS::OS()
{
	sigset_t sigmask;
	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &sigmask, nullptr);

	struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGCHLD, &sa, nullptr);
}

OS::~OS()
{
}

void OS::destroy()
{
	T_SAFE_RELEASE(this);
}

}
