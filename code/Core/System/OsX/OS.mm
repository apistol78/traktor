/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <CoreFoundation/CFBundle.h>
#include <Foundation/Foundation.h>
#if !defined(__IOS__)
#	include <crt_externs.h>
#	include <mach-o/dyld.h>
#else
#	import <UIKit/UIKit.h>
#endif
#include <glob.h>
#include <pwd.h>
#include <spawn.h>
#include <stdlib.h>
#include <unistd.h>
#include <mach/mach_host.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syslimits.h>
#include "Core/Io/FileSystem.h"
#include "Core/Io/Path.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Misc/TString.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/System/Environment.h"
#include "Core/System/OS.h"
#include "Core/System/ResolveEnv.h"
#include "Core/System/OsX/ProcessOsX.h"
#include "Core/System/OsX/SharedMemoryOsX.h"

namespace traktor
{
	namespace
	{

#if defined(__IOS__)
NSString* makeNSString(const std::wstring& str)
{
	std::string mbs = wstombs(Utf8Encoding(), str);
	return [[NSString alloc] initWithCString: mbs.c_str() encoding: NSUTF8StringEncoding];
}
#endif

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.OS", OS, Object)

OS& OS::getInstance()
{
	static OS* s_instance = nullptr;
	if (!s_instance)
	{
		s_instance = new OS();
		s_instance->addRef(nullptr);
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

std::wstring OS::getName() const
{
	return L"macOS";	
}

std::wstring OS::getIdentifier() const
{
	return L"macos";
}

uint32_t OS::getCPUCoreCount() const
{
	host_basic_info_data_t hostInfo;
	mach_msg_type_number_t infoCount;
	
	infoCount = HOST_BASIC_INFO_COUNT;
	host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t)&hostInfo, &infoCount);
	
	return uint32_t(hostInfo.max_cpus);
}

Path OS::getExecutable() const
{
#if !defined(__IOS__)
	char path[1024];
	uint32_t size = sizeof(path);
	if (_NSGetExecutablePath(path, &size) == 0)
		return Path(mbstows(path));
	else
		return Path(L"");
#else
	return Path(L"");
#endif
}

std::wstring OS::getCommandLine() const
{
	return L"";
}

std::wstring OS::getComputerName() const
{
	char name[MAXHOSTNAMELEN];
	if (gethostname(name, sizeof_array(name)) != -1)
		return mbstows(name);
	return L"";
}

std::wstring OS::getCurrentUser() const
{
	passwd* pwd = getpwuid(geteuid());
	if (!pwd)
		return L"";

	const char* who = pwd->pw_name;
	if (!who)
		return L"";

	return mbstows(who);
}

std::wstring OS::getUserHomePath() const
{
    glob_t* globbuf = (glob_t*)alloca(sizeof(glob_t));
	char path[] = { "~" };
	
	if (glob(path, GLOB_TILDE, NULL, globbuf) == 0)
	{
		char* ep = globbuf->gl_pathv[0];
		return mbstows(ep);
	}
	
	return L"";
}

std::wstring OS::getUserApplicationDataPath() const
{
	return getUserHomePath();
}

std::wstring OS::getWritableFolderPath() const
{
#if defined(__IOS__)
	NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString* documentsDirectory = [paths objectAtIndex: 0];
	return mbstows([documentsDirectory UTF8String]);
#else
	return getUserHomePath() + L"/Library";
#endif
}

bool OS::openFile(const std::wstring& file) const
{
#if !defined(__IOS__)
	system(("open " + wstombs(file)).c_str());
	return true;
#else
	NSString* fs = makeNSString(file);
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString: fs]];
	[fs release];	
    return true;
#endif
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
	return bool(setenv(
		wstombs(name).c_str(),
		wstombs(value).c_str(),
		1
	) == 0);
}

Ref< Environment > OS::getEnvironment() const
{
	Ref< Environment > env = new Environment();
	
#if !defined(__IOS__)
	char** environ = *_NSGetEnviron();
	for (char** e = environ; *e; ++e)
	{
		std::wstring pair(mbstows(*e));
		size_t p = pair.find('=');
		if (p != pair.npos)
		{
			std::wstring key = pair.substr(0, p);
			std::wstring value = pair.substr(p + 1);
			env->set(key, value);
		}
	}
#endif

	return env;
}

bool OS::getEnvironment(const std::wstring& name, std::wstring& outValue) const
{
	outValue = L"";

	// Get values from Info.plist.
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	if (mainBundle)
	{
		bool gotValue = false;
		
		if (name == L"BUNDLE_PATH")	// /path-to-bundle/MyApplication.app
		{
			if (mainBundle)
			{
				CFURLRef appUrl = CFBundleCopyBundleURL(mainBundle);
				if (appUrl)
				{
					char bundlePath[PATH_MAX];
					CFURLGetFileSystemRepresentation(appUrl, TRUE, (uint8_t*)bundlePath, PATH_MAX);
					CFRelease(appUrl);
				
					outValue = mbstows(bundlePath);
					gotValue = true;
				}
			}
		}
		
		if (gotValue)
			return true;

		// Try to get value from bundle.
		CFStringRef keyStr = CFStringCreateWithCString(kCFAllocatorDefault, wstombs(name).c_str(), kCFStringEncodingMacRoman);
		if (keyStr)
		{
			CFStringRef valueRef = (CFStringRef)CFBundleGetValueForInfoDictionaryKey(mainBundle, keyStr);
			if (valueRef)
			{
				const char* valueStr = CFStringGetCStringPtr(valueRef, kCFStringEncodingMacRoman);
				if (valueStr)
				{
					outValue = mbstows(valueStr);
					
					// \hack Replace "\$" sequences with plain "$" as Info.plist preprocessor
					// seems to intercept and remove those.
					outValue = replaceAll(outValue, L"\\$", L"$");
					
					gotValue = true;
				}
			}
			CFRelease(keyStr);
		}
		
		if (gotValue)
			return true;
	}
	
	// User home path.
	if (name == L"HOME_PATH")
	{
		outValue = getUserHomePath();
		return true;
	}

	// Ordinary variables; read from process environment.
	const char* env = getenv(wstombs(name).c_str());
	if (!env)
		return false;
		
	outValue = mbstows(env);
	return true;
}

Ref< IProcess > OS::execute(
	const std::wstring& commandLine,
	const Path& workingDirectory,
	const Environment* env,
	uint32_t flags
) const
{
#if !defined(__IOS__)

	posix_spawn_file_actions_t* fileActions = 0;
	char cwd[4096];
	AlignedVector< char* > envv;
	AlignedVector< char* > argv;
	int err;
	pid_t pid;
	int childStdOut[2] = { 0 };
	int childStdErr[2] = { 0 };

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

	// Convert working directory into absolute path.
	Path awd = FileSystem::getInstance().getAbsolutePath(workingDirectory);
	strcpy(cwd, wstombs(awd.getPathNameNoVolume()).c_str());

	// Start with bash if executing shell script.
	if (endsWith(executable, L".sh"))
		argv.push_back(strdup("/bin/sh"));

	// Convert all arguments into utf-8.
	argv.push_back(strdup(wstombs(executable).c_str()));
	for (auto it = resolvedArguments.begin() + 1; it != resolvedArguments.end(); ++it)
		argv.push_back(strdup(wstombs(*it).c_str()));

	// Convert environment variables; don't pass "DYLIB_LIBRARY_PATH" along as we
	// don't want child process searching our products by default.
	if (env)
	{
		for (auto it : env->get())
		{
			if (it.first != L"DYLD_LIBRARY_PATH")
				envv.push_back(strdup(wstombs(it.first + L"=" + it.second).c_str()));
		}
	}
	else
	{
		Ref< Environment > env2 = getEnvironment();
		for (auto it : env2->get())
		{
			if (it.first != L"DYLD_LIBRARY_PATH")
				envv.push_back(strdup(wstombs(it.first + L"=" + it.second).c_str()));
		}
	}

	// Terminate argument and environment vectors.
	envv.push_back(nullptr);
	argv.push_back(nullptr);

	// Redirect standard IO.
	if ((flags & EfRedirectStdIO) != 0)
	{
		pipe(childStdOut);
		pipe(childStdErr);

		fileActions = new posix_spawn_file_actions_t;
		posix_spawn_file_actions_init(fileActions);
		posix_spawn_file_actions_addchdir_np(fileActions, cwd);
		posix_spawn_file_actions_adddup2(fileActions, childStdOut[1], STDOUT_FILENO);
		posix_spawn_file_actions_addclose(fileActions, childStdOut[0]);
		posix_spawn_file_actions_adddup2(fileActions, childStdErr[1], STDERR_FILENO);
		posix_spawn_file_actions_addclose(fileActions, childStdErr[0]);

		// Spawn process.
		err = posix_spawn(&pid, argv[0], fileActions, 0, argv.ptr(), envv.ptr());
	}
	else
	{
		fileActions = new posix_spawn_file_actions_t;
		posix_spawn_file_actions_init(fileActions);
		posix_spawn_file_actions_addchdir_np(fileActions, cwd);

		// Spawn process.
		err = posix_spawn(&pid, argv[0], fileActions, 0, argv.ptr(), envv.ptr());
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
	
	if (err != 0)
	{
		posix_spawn_file_actions_destroy(fileActions);
		delete fileActions;
		return nullptr;
	}

	return new ProcessOsX(pid, fileActions, childStdOut[0], childStdErr[0]);
	
#else
	return nullptr;
#endif
}

Ref< ISharedMemory > OS::createSharedMemory(const std::wstring& name, uint32_t size) const
{
	Ref< SharedMemoryOsX > shm = new SharedMemoryOsX();
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
	if (executable == L"blender")
	{
		outPath = L"/Applications/Blender.app/Contents/MacOS/Blender";
		return true;
	}
	return false;
}

bool OS::getAssociatedExecutable(const std::wstring& extension, Path& outPath) const
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
	T_SAFE_RELEASE(this);
}

}
