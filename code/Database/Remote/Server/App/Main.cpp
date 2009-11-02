#include <signal.h>
#include "Xml/XmlDeserializer.h"
#include "Database/Remote/Server/ConnectionManager.h"
#include "Database/Remote/Server/Configuration.h"
#include "Database/Remote/Server/RemoteDatabaseService.h"
#include "Net/Network.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Stream.h"
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
	CommandLine cmdLine(argc, argv);
#else
int WinMain(HINSTANCE, HINSTANCE, LPTSTR cmdLine, int showCmd)
{
	const char* argv[] = { "" };
	CommandLine cmdLine(1, argv);
#endif

	traktor::log::info << L"Traktor Remote Database Server v1.0" << Endl;
	if (cmdLine.hasOption('h'))
	{
		traktor::log::info << L"Usage: Traktor.Database.Remote.Server.App -c=[configuration] -h" << Endl;
		return 1;
	}
	
	std::wstring configurationFile = L"Traktor.Database.Remote.Server.config";
	if (cmdLine.hasOption('s'))
		configurationFile = cmdLine.getOption('c').getString();

	Ref< Stream > file = FileSystem::getInstance().open(configurationFile, File::FmRead);
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
	Ref< db::ConnectionManager > connectionManager = gc_new< db::ConnectionManager >();
	if (!connectionManager->create(configuration))
	{
		traktor::log::error << "Unable to create connection manager" << Endl;
		return 1;
	}
	
	// Initialize network discovery.
	Ref< net::DiscoveryManager > discoveryManager = gc_new< net::DiscoveryManager >();
	if (discoveryManager->create())
	{
		Ref< db::RemoteDatabaseService > service = gc_new< db::RemoteDatabaseService >(
			L"localhost",
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
