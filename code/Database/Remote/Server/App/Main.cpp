/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <signal.h>
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Compact/CompactDatabase.h"
#include "Database/Local/LocalDatabase.h"
#include "Database/Remote/Server/ConnectionManager.h"
#include "Net/Network.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Discovery/NetworkService.h"
#include "Net/Stream/StreamServer.h"
#include "Xml/XmlDeserializer.h"

using namespace traktor;

static bool g_running = true;

#if defined(_WIN32)
BOOL WINAPI controlAbortCallback(DWORD dwCtrlType)
{
	g_running = false;
	return dwCtrlType == CTRL_C_EVENT;
}
#endif

void signalAbortCallback(int signal)
{
	g_running = false;
}

int main(int argc, const char** argv)
{
	T_FORCE_LINK_REF(db::CompactDatabase);
	T_FORCE_LINK_REF(db::LocalDatabase);

	CommandLine cmdLine(argc, argv);

	traktor::log::info << L"Traktor Remote Database Server v1.0.2" << Endl;
	if (cmdLine.hasOption('h', L"help"))
	{
		traktor::log::info << L"Usage: Traktor.Database.Remote.Server.App -c|-config=[configuration] -h|-help" << Endl;
		return 1;
	}
	
	std::wstring configurationFile = L"Traktor.Database.Remote.Server.config";
	if (cmdLine.hasOption('c', L"config"))
		configurationFile = cmdLine.getOption('c', L"config").getString();

	Ref< traktor::IStream > file = FileSystem::getInstance().open(configurationFile, File::FmRead);
	if (!file)
	{
		traktor::log::error << L"Unable to open configuration \"" << configurationFile << L"\"" << Endl;
		return 1;
	}

	//Ref< db::Configuration > configuration = xml::XmlDeserializer(file).readObject< db::Configuration >();
	//if (!configuration)
	//{
	//	traktor::log::error << L"Unable to read configuration \"" << configurationFile << L"\"" << Endl;
	//	return 1;
	//}

	file->close();

	if (!net::Network::initialize())
	{
		traktor::log::error << L"Unable to initialize network" << Endl;
		return 1;
	}

	// Create stream server.
	Ref< net::StreamServer > streamServer = new net::StreamServer();
	streamServer->create();

	// Initialize database connection manager.
	Ref< db::ConnectionManager > connectionManager = new db::ConnectionManager(streamServer);
	//if (!connectionManager->create(configuration))
	{
		traktor::log::error << "Unable to create connection manager" << Endl;
		return 1;
	}
	
	// Initialize network discovery.
	Ref< net::DiscoveryManager > discoveryManager = new net::DiscoveryManager();
	if (discoveryManager->create(net::MdPublishServices))
	{
		net::SocketAddressIPv4::Interface itf;
		if (!net::SocketAddressIPv4::getBestInterface(itf))
		{
			traktor::log::error << L"Unable to get interfaces" << Endl;
			return 1;
		}

		Ref< PropertyGroup > properties = new PropertyGroup();
		properties->setProperty< PropertyString >(L"Host", itf.addr->getHostName());
		//properties->setProperty< PropertyInteger >(L"Port", configuration->getListenPort());
		Ref< net::NetworkService > service = new net::NetworkService(L"Database/Server", properties);

		discoveryManager->addService(service);
	}
	else
	{
		traktor::log::warning << L"Unable to initialize network discovery" << Endl;
		discoveryManager = 0;
	}

#if defined(_WIN32)
	SetConsoleCtrlHandler(controlAbortCallback, TRUE);
#endif

	signal(SIGTERM, signalAbortCallback);
	signal(SIGABRT, signalAbortCallback);
#if defined(_WIN32)
	signal(SIGBREAK, signalAbortCallback);
#endif

	traktor::log::info << L"Server started" << Endl;

	while (g_running)
		ThreadManager::getInstance().getCurrentThread()->sleep(1000);
	
	if (discoveryManager)
	{
		discoveryManager->destroy();
		discoveryManager = 0;
	}

	traktor::log::info << L"Server stopped" << Endl;

	net::Network::finalize();
	return 0;
}
