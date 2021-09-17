#include "Avalanche/Server/Server.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Net/Network.h"

using namespace traktor;

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	if (cmdLine.hasOption('h', L"help"))
	{
		log::info << L"Traktor.Avalanche.Server.App; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;
		log::info << L"Usage:" << Endl;
		log::info << Endl;
		log::info << L"  Traktor.Avalanche.Serve.App (options)" << Endl;
		log::info << L"    -p, -port    Port number." << Endl;
		log::info << L"    -h, -help    Help" << Endl;
		return 0;
	}

	log::info << L"Traktor.Avalanche.Server.App; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;

	int32_t port = 40001;
	if (cmdLine.hasOption('p', L"port"))
		port = cmdLine.getOption('p', L"port").getInteger();

	Ref< PropertyGroup > settings = new PropertyGroup();
	settings->setProperty< PropertyInteger >(L"Avalanche.Port", port);

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
