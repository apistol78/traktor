#if defined(__LINUX__) || defined(__RPI__) || defined(__APPLE__)
#	include <signal.h>
#endif
#include "Avalanche/Server/Server.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Net/Network.h"

using namespace traktor;

#if defined(T_STATIC)
extern "C" void __module__Traktor_Core();
#endif

int main(int argc, const char** argv)
{
#if defined(T_STATIC)
	__module__Traktor_Core();
#endif

#if defined(__LINUX__) || defined(__RPI__) || defined(__APPLE__)
	struct sigaction sa = { SIG_IGN };
	sigaction(SIGPIPE, &sa, nullptr);
#endif

	CommandLine cmdLine(argc, argv);

	log::info << L"Traktor.Avalanche.Server.App; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;

	if (cmdLine.hasOption('h', L"help"))
	{
		log::info << L"Usage: Traktor.Avalanche.Server.App (options)" << Endl;
		log::info << L"    -m, -master  Master node." << Endl;
		log::info << L"    -p, -port    Port number (default 40001)." << Endl;
		log::info << L"    -h, -help    Help" << Endl;
		return 0;
	}

	int32_t port = 40001;
	if (cmdLine.hasOption('p', L"port"))
		port = cmdLine.getOption('p', L"port").getInteger();

	Ref< PropertyGroup > settings = new PropertyGroup();
	settings->setProperty< PropertyInteger >(L"Avalanche.Port", port);
	settings->setProperty< PropertyBoolean >(L"Avalanche.Master", cmdLine.hasOption('m', L"master"));

	if (!net::Network::initialize())
	{
		log::error << L"Unable to initialize networking." << Endl;
		return 1;
	}

	Ref< avalanche::Server > server = new avalanche::Server();
	if (!server->create(settings))
		return 1;

	while (server->update())
		;

	server->destroy();
	server = nullptr;

	net::Network::finalize();
	return 0;
}
