#include <signal.h>
#include "Xml/XmlDeserializer.h"
#include "Database/Compact/CompactDatabase.h"
#include "Database/Local/LocalDatabase.h"
#include "Database/Remote/Server/ConnectionManager.h"
#include "Database/Remote/Server/Configuration.h"
#include "Database/Remote/Server/RemoteDatabaseService.h"
#include "Net/Network.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"

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

#if !defined(WINCE)
int main(int argc, const char** argv)
{
	T_FORCE_LINK_REF(db::CompactDatabase);
	T_FORCE_LINK_REF(db::LocalDatabase);

	CommandLine cmdLine(argc, argv);
#else
int WinMain(HINSTANCE, HINSTANCE, LPTSTR cmdLine, int showCmd)
{
	const char* argv[] = { "" };
	CommandLine cmdLine(1, argv);
#endif

	traktor::log::info << L"Traktor Remote Database Server v1.0.1" << Endl;
	if (cmdLine.hasOption('h'))
	{
		traktor::log::info << L"Usage: Traktor.Database.Remote.Server.App -c=[configuration] -h" << Endl;
		return 1;
	}
	
	std::wstring configurationFile = L"Traktor.Database.Remote.Server.config";
	if (cmdLine.hasOption('c'))
		configurationFile = cmdLine.getOption('c').getString();

	Ref< traktor::IStream > file = FileSystem::getInstance().open(configurationFile, File::FmRead);
	if (!file)
	{
		traktor::log::error << L"Unable to open configuration \"" << configurationFile << L"\"" << Endl;
		return 1;
	}

	Ref< db::Configuration > configuration = xml::XmlDeserializer(file).readObject< db::Configuration >();
	if (!configuration)
	{
		traktor::log::error << L"Unable to read configuration \"" << configurationFile << L"\"" << Endl;
		return 1;
	}

	file->close();

	if (!net::Network::initialize())
	{
		traktor::log::error << L"Unable to initialize network" << Endl;
		return 1;
	}

	// Initialize database connection manager.
	Ref< db::ConnectionManager > connectionManager = new db::ConnectionManager();
	if (!connectionManager->create(configuration))
	{
		traktor::log::error << "Unable to create connection manager" << Endl;
		return 1;
	}
	
	// Initialize network discovery.
	Ref< net::DiscoveryManager > discoveryManager = new net::DiscoveryManager();
	if (discoveryManager->create(true))
	{
		RefArray< net::SocketAddressIPv4 > interfaces = net::SocketAddressIPv4::getInterfaces();
		T_ASSERT (!interfaces.empty());

		std::wstring host = interfaces[0]->getHostName();

		Ref< db::RemoteDatabaseService > service = new db::RemoteDatabaseService(
			host,
			configuration->getListenPort()
		);

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
	{
		if (!connectionManager->update())
			g_running = false;
	}
	
	if (discoveryManager)
	{
		discoveryManager->destroy();
		discoveryManager = 0;
	}

	traktor::log::info << L"Server stopped" << Endl;

	net::Network::finalize();
	return 0;
}
