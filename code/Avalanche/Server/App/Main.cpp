/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#if defined(_WIN32)
#	include <Windows.h>
#elif defined(__LINUX__) || defined(__RPI__) || defined(__APPLE__)
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <signal.h>
#	include <stdio.h>
#	include <stdlib.h>
#	include <fcntl.h>
#	include <errno.h>
#	include <unistd.h>
#	include <syslog.h>
#	include <string.h>
#endif
#include "Avalanche/Server/Server.h"
#include "Core/Io/AnsiEncoding.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Net/Network.h"

using namespace traktor;

#if defined(T_STATIC)
extern "C" void __module__Traktor_Core();
#endif

#if defined(_WIN32)

const wchar_t* serviceName = L"Traktor Avalanche Service";
SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE serviceStatusHandle = 0;
std::atomic< bool > serviceRunning = true;

void WINAPI serviceControlHandler(DWORD controlCode)
{
	switch (controlCode)
	{
	case SERVICE_CONTROL_INTERROGATE:
		break;

	case SERVICE_CONTROL_SHUTDOWN:
	case SERVICE_CONTROL_STOP:
		serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);
		serviceRunning = false;
		return;

	case SERVICE_CONTROL_PAUSE:
		break;

	case SERVICE_CONTROL_CONTINUE:
		break;

	default:
		break;
	}

	SetServiceStatus(serviceStatusHandle, &serviceStatus);
}

void WINAPI serviceMain(DWORD argc, TCHAR* argv[])
{
	CommandLine cmdLine((int)argc, (const wchar_t**)argv);

	// Initialise service status.
	serviceStatus.dwServiceType = SERVICE_WIN32;
	serviceStatus.dwCurrentState = SERVICE_STOPPED;
	serviceStatus.dwControlsAccepted = 0;
	serviceStatus.dwWin32ExitCode = NO_ERROR;
	serviceStatus.dwServiceSpecificExitCode = NO_ERROR;
	serviceStatus.dwCheckPoint = 0;
	serviceStatus.dwWaitHint = 0;
	serviceStatusHandle = RegisterServiceCtrlHandler(serviceName, serviceControlHandler);

	if (serviceStatusHandle)
	{
		// Service is starting.
		serviceStatus.dwCurrentState = SERVICE_START_PENDING;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);

		int32_t port = 40001;
		if (cmdLine.hasOption('p', L"port"))
			port = cmdLine.getOption('p', L"port").getInteger();

		Ref< PropertyGroup > settings = new PropertyGroup();
		settings->setProperty< PropertyInteger >(L"Avalanche.Port", port);
		settings->setProperty< PropertyBoolean >(L"Avalanche.Master", cmdLine.hasOption('m', L"master"));
		settings->setProperty< PropertyString >(L"Avalanche.Path", cmdLine.getOption('d', L"dictionary-path").getString());
		settings->setProperty< PropertyInteger >(L"Avalanche.MemoryBudget", cmdLine.getOption('b', L"memory-budget").getInteger());

		if (!net::Network::initialize())
		{
			log::error << L"Unable to initialize networking." << Endl;
			serviceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
			serviceStatus.dwCurrentState = SERVICE_STOPPED;
			SetServiceStatus(serviceStatusHandle, &serviceStatus);
			return;
		}

		Ref< avalanche::Server > server = new avalanche::Server();
		if (!server->create(settings))
		{
			log::error << L"Unable to create server." << Endl;
			serviceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
			serviceStatus.dwCurrentState = SERVICE_STOPPED;
			SetServiceStatus(serviceStatusHandle, &serviceStatus);
			return;
		}

		// Running
		serviceStatus.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		serviceStatus.dwCurrentState = SERVICE_RUNNING;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);

		while (serviceRunning)
		{
			if (!server->update())
				break;
		}

		// Service was stopped.
		serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);

		safeDestroy(server);
		net::Network::finalize();

		// Service is now stopped.
		serviceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);
	}
}

bool installService(const std::wstring& arguments)
{
	TCHAR path[_MAX_PATH + 1];
	if (GetModuleFileName(0, path, sizeof_array(path)) == 0)
	{
		log::error << L"Unable to get module filename." << Endl;
		return false;
	}

	std::wstring imagePath;
	if (!arguments.empty())
		imagePath = str(L"\"%s\" %s", path, arguments.c_str());
	else
		imagePath = str(L"\"%s\"", path);

	SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);
	if (!serviceControlManager)
	{
		log::error << L"Unable to open SCM manager." << Endl;
		return false;
	}

	SC_HANDLE service = CreateService(
		serviceControlManager,
		serviceName,
		serviceName,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START,
		SERVICE_ERROR_IGNORE,
		imagePath.c_str(),
		0,
		0,
		0,
		nullptr, // L"NT AUTHORITY\\NetworkService",
		0
	);
	if (service)
		CloseServiceHandle(service);
	else
		log::error << L"Unable to create service." << Endl;

	CloseServiceHandle(serviceControlManager);
	return service != NULL;
}

bool uninstallService()
{
	SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CONNECT);
	if (!serviceControlManager)
	{
		log::error << L"Unable to open SCM manager." << Endl;
		return false;
	}

	SC_HANDLE service = OpenService(serviceControlManager, serviceName, SERVICE_QUERY_STATUS | DELETE);
	if (!service)
	{
		log::error << L"Unable to open service." << Endl;
		CloseServiceHandle(serviceControlManager);
		return false;
	}

	SERVICE_STATUS serviceStatus;
	if (!QueryServiceStatus(service, &serviceStatus))
	{
		log::error << L"Unable to query service status." << Endl;
		CloseServiceHandle(serviceControlManager);
		return false;
	}
	
	bool deleted = false;
	if (serviceStatus.dwCurrentState == SERVICE_STOPPED)
		deleted = (DeleteService(service) != FALSE);

	CloseServiceHandle(service);
	CloseServiceHandle(serviceControlManager);
	return deleted;
}

#endif

bool g_running = true;

#if defined(__LINUX__) || defined(__RPI__) || defined(__APPLE__)
void abortHandler(int s)
{
	g_running = false;
}
#endif

int main(int argc, const char** argv)
{
#if defined(T_STATIC)
	__module__Traktor_Core();
#endif

#if defined(__LINUX__) || defined(__RPI__) || defined(__APPLE__)
	{
		struct sigaction sa = { SIG_IGN };
		sigaction(SIGPIPE, &sa, nullptr);
	}
	{
		struct sigaction sa;
		sa.sa_handler = abortHandler;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sigaction(SIGINT, &sa, nullptr);
	}
#endif

	CommandLine cmdLine(argc, argv);

	log::info << L"Traktor.Avalanche.Server.App " << avalanche::Server::c_majorVersion << L"." << avalanche::Server::c_minorVersion << L"; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;

	int32_t port = 40001;
	if (cmdLine.hasOption('p', L"port"))
		port = cmdLine.getOption('p', L"port").getInteger();

	if (cmdLine.hasOption('h', L"help") || port <= 0)
	{
		log::info << L"Usage: Traktor.Avalanche.Server.App (options)" << Endl;
		log::info << L"    -m, -master           Master node." << Endl;
		log::info << L"    -p, -port             Port number (default 40001)." << Endl;
		log::info << L"    -d, -dictionary-path  Path to dictionary blobs." << Endl;
		log::info << L"    -b, -memory-budget    Memory budget in GiB (default 8)." << Endl;
#if defined(_WIN32)
		log::info << L"    -install-service      Install as NT service." << Endl;
		log::info << L"    -uninstall-service    Uninstall as NT service." << Endl;
#elif defined(__LINUX__) || defined(__RPI__)
		log::info << L"    -daemon               Launch as a daemon." << Endl;
#endif
		log::info << L"    -h, -help             Help" << Endl;
		return 0;
	}

#if defined(_WIN32)
	if (cmdLine.hasOption(L"install-service"))
	{
		std::wstring arguments = str(L"-run-service -p=%d", port);
		if (cmdLine.hasOption('m', L"master"))
			arguments += L" -m";

		if (installService(arguments))
		{
			log::info << L"Installed Traktor Avalanche NT service successfully." << Endl;
			return 0;
		}
		else
		{
			log::error << L"Unable to install Traktor Avalanche NT service." << Endl;
			return 1;
		}
	}
	if (cmdLine.hasOption(L"uninstall-service"))
	{
		if (uninstallService())
		{
			log::info << L"Uninstalled Traktor Avalanche NT service successfully." << Endl;
			return 0;
		}
		else
		{
			log::error << L"Unable to uninstall Traktor Avalanche NT service." << Endl;
			return 1;
		}
	}
	if (cmdLine.hasOption(L"run-service"))
	{
		SERVICE_TABLE_ENTRY serviceTable[] =
		{
			{ (LPWSTR)serviceName, serviceMain },
			{ 0, 0 }
		};
		StartServiceCtrlDispatcher(serviceTable);
		return 0;
	}
#elif defined(__LINUX__) || defined(__RPI__)
	if (cmdLine.hasOption(L"daemon"))
	{
		pid_t pid = fork();
		if (pid < 0)
		{
			log::error << L"Unable to fork into daemon." << Endl;
			return 1;
		}
		if (pid > 0)
		{
			log::info << L"Daemon started succesfully (pid " << (int32_t)pid << L")." << Endl;
			return 0;
		}

		umask(0);

		pid_t sid = setsid();
		if (sid < 0)
			return 1;

		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);

		// Write daemon pid to run file.
		Ref< IStream > pf = FileSystem::getInstance().open(L"/var/run/Traktor.Avalanche.Server.App.pid", File::FmWrite);
		if (pf)
		{
			FileOutputStream(pf, new AnsiEncoding()) << (uint32_t)sid << Endl;
			pf->close();
			pf = nullptr;
		}
	}
#endif

	Ref< PropertyGroup > settings = new PropertyGroup();
	settings->setProperty< PropertyInteger >(L"Avalanche.Port", port);
	settings->setProperty< PropertyBoolean >(L"Avalanche.Master", cmdLine.hasOption('m', L"master"));
	settings->setProperty< PropertyString >(L"Avalanche.Path", cmdLine.getOption('d', L"dictionary-path").getString());
	settings->setProperty< PropertyInteger >(L"Avalanche.MemoryBudget", cmdLine.getOption('b', L"memory-budget").getInteger());
	if (!net::Network::initialize())
	{
		log::error << L"Unable to initialize networking." << Endl;
		return 1;
	}

	Ref< avalanche::Server > server = new avalanche::Server();
	if (!server->create(settings))
		return 1;

	while (g_running && server->update())
		;

	server->destroy();
	server = nullptr;

	net::Network::finalize();

	log::info << L"Bye." << Endl;
	return 0;
}
