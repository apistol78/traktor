/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Misc/TString.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/System/Environment.h"
#include "Core/System/Linux/ProcessLinux.h"
#include "Core/System/Linux/SharedMemoryLinux.h"
#include "Core/System/PipeReader.h"
#include "Core/System/ResolveEnv.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <pwd.h>
#include <spawn.h>
#include <sys/inotify.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace traktor
{
namespace
{

void handle_sigchld(int sig)
{
}

std::wstring readWholeFile(const std::wstring& path)
{
	Ref< IStream > s = FileSystem::getInstance().open(path, File::FmRead);
	if (!s)
		return L"";

	std::string content;
	char buf[4096];
	int64_t n;

	while ((n = s->read(buf, sizeof(buf))) > 0)
		content.append(buf, n);

	return mbstows(content);
}

std::wstring desktopEntryValue(const std::wstring& content, const std::wstring& key)
{
	bool inDesktopEntry = false;
	const std::wstring prefix = key + L"=";
	for (auto line : StringSplit< std::wstring >(content, L"\n"))
	{
		while (!line.empty() && (line.back() == L'\r' || line.back() == L' ' || line.back() == L'\t'))
			line.pop_back();
		if (line.empty() || line[0] == L'#')
			continue;
		if (line[0] == L'[')
		{
			inDesktopEntry = (line == L"[Desktop Entry]");
			continue;
		}
		if (inDesktopEntry && startsWith(line, prefix))
			return line.substr(prefix.size());
	}
	return std::wstring();
}

std::wstring execLineExecutable(const std::wstring& execLine)
{
	size_t start = 0;
	while (start < execLine.size() && (execLine[start] == L' ' || execLine[start] == L'\t'))
		++start;
	size_t end = start;
	while (end < execLine.size() && execLine[end] != L' ' && execLine[end] != L'\t')
		++end;
	std::wstring exe = execLine.substr(start, end - start);
	if (exe.size() >= 2 && exe.front() == L'"' && exe.back() == L'"')
		exe = exe.substr(1, exe.size() - 2);
	return exe;
}

bool executableExists(const std::wstring& execLine)
{
	const std::wstring exe = execLineExecutable(execLine);
	if (exe.empty())
		return false;
	if (exe[0] == L'/')
		return access(wstombs(exe).c_str(), X_OK) == 0;
	const char* path = getenv("PATH");
	if (!path)
		return false;
	for (auto p : StringSplit< std::wstring >(mbstows(path), L":"))
	{
		if (access((wstombs(p) + "/" + wstombs(exe)).c_str(), X_OK) == 0)
			return true;
	}
	return false;
}

bool spawnDesktopExec(const std::wstring& execLine, const std::wstring& file)
{
	std::wstring cmd;
	bool substituted = false;
	for (size_t i = 0; i < execLine.size();)
	{
		if (execLine[i] == L'%' && i + 1 < execLine.size())
		{
			const wchar_t p = execLine[i + 1];
			if (p == L'f' || p == L'F' || p == L'u' || p == L'U')
			{
				cmd += L"\"" + file + L"\"";
				substituted = true;
				i += 2;
				continue;
			}
			if (p == L'i' || p == L'c' || p == L'k' || p == L'd' || p == L'D' || p == L'n' || p == L'N' || p == L'v' || p == L'm')
			{
				i += 2;
				continue;
			}
			if (p == L'%')
			{
				cmd += L'%';
				i += 2;
				continue;
			}
		}
		cmd += execLine[i];
		++i;
	}
	if (!substituted)
		cmd += L" \"" + file + L"\"";
	const std::string shellCmd = "(" + wstombs(cmd) + ") >/dev/null 2>&1 &";
	return system(shellCmd.c_str()) == 0;
}

AlignedVector< std::wstring > desktopSearchDirs()
{
	AlignedVector< std::wstring > dirs;
	if (const char* h = getenv("HOME"))
	{
		const std::wstring home = mbstows(h);
		dirs.push_back(home + L"/.local/share/applications");
		dirs.push_back(home + L"/.local/share/flatpak/exports/share/applications");
	}
	dirs.push_back(L"/var/lib/flatpak/exports/share/applications");
	dirs.push_back(L"/usr/local/share/applications");
	dirs.push_back(L"/usr/share/applications");
	if (const char* xdg = getenv("XDG_DATA_DIRS"))
	{
		for (auto p : StringSplit< std::wstring >(mbstows(xdg), L":"))
			dirs.push_back(p + L"/applications");
	}
	return dirs;
}

std::wstring findDesktopFile(const std::wstring& name)
{
	for (const auto& dir : desktopSearchDirs())
	{
		const std::wstring full = dir + L"/" + name;
		if (access(wstombs(full).c_str(), R_OK) == 0)
			return full;
	}
	return std::wstring();
}

std::wstring findFlatpakDesktopForMime(const std::wstring& mimeType)
{
	AlignedVector< std::wstring > flatpakDirs;
	if (const char* h = getenv("HOME"))
		flatpakDirs.push_back(mbstows(h) + L"/.local/share/flatpak/exports/share/applications");
	flatpakDirs.push_back(L"/var/lib/flatpak/exports/share/applications");

	for (const auto& dir : flatpakDirs)
	{
		RefArray< File > files = FileSystem::getInstance().find(dir + L"/*.desktop");
		for (auto f : files)
		{
			const std::wstring path = f->getPath().getPathName();
			const std::wstring content = readWholeFile(path);
			const std::wstring mimeLine = desktopEntryValue(content, L"MimeType");
			if (mimeLine.empty())
				continue;
			for (auto m : StringSplit< std::wstring >(mimeLine, L";"))
			{
				if (m == mimeType)
					return path;
			}
		}
	}
	return std::wstring();
}

std::wstring xdgMimeQuery(const std::string& args)
{
	FILE* p = popen(("xdg-mime " + args + " 2>/dev/null").c_str(), "r");
	if (!p)
		return std::wstring();
	char buf[512] = { 0 };
	const bool ok = fgets(buf, sizeof(buf), p) != nullptr;
	pclose(p);
	if (!ok)
		return std::wstring();
	std::string s = buf;
	while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' ' || s.back() == '\t'))
		s.pop_back();
	return mbstows(s);
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
	return system(("xdg-open \"" + wstombs(file) + "\"").c_str()) != 0;
}

bool OS::editFile(const std::wstring& file) const
{
	const std::string mbFile = wstombs(file);

	// Resolve the file's MIME type and its registered default handler. If the
	// handler's binary is missing (e.g. a stale desktop entry left over from a
	// package that has been replaced by a flatpak), fall back to scanning the
	// flatpak export directories for an application that declares support for
	// this MIME type.
	std::wstring execLine;
	const std::wstring mimeType = xdgMimeQuery("query filetype \"" + mbFile + "\"");
	if (!mimeType.empty())
	{
		const std::wstring defaultDesktop = xdgMimeQuery("query default \"" + wstombs(mimeType) + "\"");
		if (!defaultDesktop.empty())
		{
			const std::wstring desktopPath = findDesktopFile(defaultDesktop);
			if (!desktopPath.empty())
			{
				const std::wstring content = readWholeFile(desktopPath);
				std::wstring candidate = desktopEntryValue(content, L"Exec");
				if (!candidate.empty() && executableExists(candidate))
					execLine = std::move(candidate);
			}
		}

		if (execLine.empty())
		{
			const std::wstring flatpakDesktop = findFlatpakDesktopForMime(mimeType);
			if (!flatpakDesktop.empty())
			{
				const std::wstring content = readWholeFile(flatpakDesktop);
				execLine = desktopEntryValue(content, L"Exec");
			}
		}
	}

	if (!execLine.empty())
		return spawnDesktopExec(execLine, file);

	return system(("xdg-open \"" + mbFile + "\" >/dev/null 2>&1 &").c_str()) == 0;
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
					1) == 0);
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
	uint32_t flags) const
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
		for (auto it : env->get())
			envv.push_back(strdup(wstombs(it.first + L"=" + it.second).c_str()));

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
		if (arg)
			free(arg);
	for (auto env : envv)
		if (env)
			free(env);

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
		childStdErr[0]);
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

bool OS::whereIs(const std::wstring& executable, std::wstring& outPath) const
{
	std::wstring paths;

	// Get system "PATH" environment variable.
	if (getEnvironment(L"PATH", paths))
	{
		// Try to locate binary in any of the paths specified in "PATH".
		for (auto path : StringSplit< std::wstring >(paths, L";:,"))
		{
			Ref< File > file = FileSystem::getInstance().get(path + L"/" + executable);
			if (file)
			{
				outPath = L"\"" + file->getPath().getPathNameOS() + L"\"";
				return true;
			}
		}
	}

	// Check if it's a flatpak application.
	Ref< IProcess > fp = execute(L"flatpak list --app", L"", nullptr, OS::EfRedirectStdIO);
	if (fp)
	{
		PipeReader reader(fp->getPipeStream(IProcess::SpStdOut));

		AlignedVector< std::wstring > tkns;
		PipeReader::Result result;
		std::wstring str;

		while ((result = reader.readLine(str)) != PipeReader::RtEnd)
		{
			if (str.empty())
				continue;

			tkns.resize(0);
			Split< std::wstring >::any(str, L" \t", tkns);

			if (tkns.size() >= 2 && compareIgnoreCase(tkns[0], executable) == 0)
			{
				// Found flatpak identifier.
				outPath = L"flatpak run " + tkns[1];
				return true;
			}
		}		
	}

	return false;
}

bool OS::getAssociatedExecutable(const std::wstring& extension, std::wstring& outPath) const
{
	return false;
}

bool OS::waitUntilAnyFileChange(const Path& path, int32_t timeout) const
{
	const std::string osp = wstombs(FileSystem::getInstance().getAbsolutePath(path).getPathNameOS());

	const int fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
	if (fd < 0)
		return false;

	// inotify does not support recursive watches; walk the tree and add a watch per directory.
	AlignedVector< std::string > pending;
	pending.push_back(osp);
	while (!pending.empty())
	{
		const std::string dir = pending.back();
		pending.pop_back();

		if (inotify_add_watch(fd, dir.c_str(), IN_MODIFY | IN_CLOSE_WRITE) < 0)
			continue;

		DIR* d = opendir(dir.c_str());
		if (!d)
			continue;

		struct dirent* entry;
		while ((entry = readdir(d)) != nullptr)
		{
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;

			const std::string sub = dir + "/" + entry->d_name;
			struct stat st;
			if (lstat(sub.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
				pending.push_back(sub);
		}
		closedir(d);
	}

	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLIN;
	const int pollResult = poll(&pfd, 1, timeout);
	const bool change = (pollResult > 0 && (pfd.revents & POLLIN) != 0);

	close(fd);
	return change;
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
