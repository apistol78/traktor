#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/System/OS.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Editor/IPipeline.h"
#include "Editor/Pipeline/AgentBuild.h"
#include "Editor/Pipeline/AgentConnect.h"
#include "Editor/Pipeline/AgentStatus.h"
#include "Editor/Pipeline/PipelineFactory.h"
#include "Editor/Pipeline/PipelineSettings.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Net/Network.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/TcpSocket.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Discovery/NetworkService.h"
#include "Pipeline/Agent/App/PipelineBuilderWrapper.h"
#include "Xml/XmlDeserializer.h"

using namespace traktor;

const uint16_t c_defaultAgentPort = 39000;
const int32_t c_defaultAgentCount = 4;

void threadProcessClient(
	Ref< editor::AgentConnect > agentConnect,
	Ref< net::BidirectionalObjectTransport > transport,
	Ref< db::Database > sourceDatabase,
	Ref< db::Database > outputDatabase
)
{
	// Create pipeline environment.
	Ref< editor::PipelineSettings > pipelineSettings = new editor::PipelineSettings(agentConnect->getSettings());
	Ref< editor::PipelineFactory > pipelineFactory = new editor::PipelineFactory(agentConnect->getSettings());
	Ref< editor::PipelineBuilderWrapper > pipelineBuilder = new editor::PipelineBuilderWrapper(
		pipelineFactory,
		transport,
		agentConnect->getHost(),
		agentConnect->getStreamServerPort(),
		sourceDatabase,
		outputDatabase
	);

	log::info << L"Ready; waiting for build items..." << Endl;

	// Agent connected and ready; wait for build items.
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	while (!currentThread->stopped() && transport->connected())
	{
		Ref< editor::AgentBuild > agentBuild;
		if (transport->recv< editor::AgentBuild >(100, agentBuild) != 1)
			continue;

		log::info << L"Received build item " << agentBuild->getOutputGuid().format() << Endl;

		const TypeInfo* pipelineType = TypeInfo::find(agentBuild->getPipelineTypeName());
		if (!pipelineType)
		{
			log::error << L"Agent build error; no such pipeline \"" << agentBuild->getPipelineTypeName() << L"\"" << Endl;
			continue;
		}

		Ref< editor::IPipeline > pipeline;
		uint32_t pipelineHash;

		if (!pipelineFactory->findPipeline(type_of(agentBuild->getSourceAsset()), pipeline, pipelineHash))
		{
			log::error << L"Agent build error; unable to get pipeline" << Endl;
			continue;
		}

		if (!is_type_a(type_of(pipeline), *pipelineType))
		{
			log::error << L"Agent build error; mismatching pipeline types" << Endl;
			continue;
		}

		log::info << L"Executing build using pipeline \"" << type_name(pipeline) << L"\"..." << Endl;

		bool result = pipeline->buildOutput(
			pipelineBuilder,
			agentBuild->getSourceAsset(),
			agentBuild->getSourceAssetHash(),
			agentBuild->getOutputPath(),
			agentBuild->getOutputGuid(),
			0,
			agentBuild->getReason()
		);

		editor::AgentStatus agentStatus(
			agentBuild->getOutputGuid(),
			result
		);
		transport->send(&agentStatus);

		if (result)
			log::info << L"Build succeeded" << Endl;
		else
			log::info << L"Build failed" << Endl;
	}

	transport->close();
	transport = 0;
}

Ref< PropertyGroup > loadProperties(const Path& pathName)
{
	Ref< PropertyGroup > settings;
	Ref< traktor::IStream > file;

#if defined(_WIN32)
	std::wstring system = L"win32";
#elif defined(__APPLE__)
	std::wstring system = L"osx";
#else   // LINUX
	std::wstring system = L"linux";
#endif

	std::wstring globalFile = pathName.getPathName();
	std::wstring systemFile = pathName.getPathNameNoExtension() + L"." + system + L"." + pathName.getExtension();
	std::wstring userFile = pathName.getPathNameNoExtension() + L"." + OS::getInstance().getCurrentUser() + L"." + pathName.getExtension();

	// Read global properties.
	if ((file = FileSystem::getInstance().open(globalFile, File::FmRead)) != 0)
	{
		settings = xml::XmlDeserializer(file).readObject< PropertyGroup >();
		file->close();

		if (!settings)
			log::error << L"Error while parsing properties \"" << globalFile << L"\"" << Endl;
		else
			T_DEBUG(L"Successfully read properties from \"" << globalFile << L"\"");
	}
	else
		log::warning << L"Unable to read global properties \"" << globalFile << L"\"" << Endl;

	// Read system properties.
	if ((file = FileSystem::getInstance().open(systemFile, File::FmRead)) != 0)
	{
		Ref< PropertyGroup > systemSettings = xml::XmlDeserializer(file).readObject< PropertyGroup >();
		file->close();

		if (systemSettings)
		{
			if (settings)
			{
				settings = settings->mergeReplace(systemSettings);
				T_ASSERT (settings);
			}
			else
				settings = systemSettings;

			T_DEBUG(L"Successfully read properties from \"" << systemFile << L"\"");
		}
		else
			log::error << L"Error while parsing properties \"" << systemFile << L"\"" << Endl;
	}

	return settings;
}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	if (cmdLine.hasOption('h', L"help"))
	{
		traktor::log::info << L"Usage: Traktor.Pipeline.Agent.App (-options)" << Endl;
		traktor::log::info << L"       -s|-settings        Settings (default \"Traktor.Editor.config\")" << Endl;
		traktor::log::info << L"       -n|-agents=<agents> Number of agent threads (default 4)" << Endl;
		traktor::log::info << L"       -h|-help            Show this help" << Endl;
		return 0;
	}

	std::wstring settingsFile = L"Traktor.Editor.config";
	if (cmdLine.hasOption('s', L"settings"))
		settingsFile = cmdLine.getOption('s', L"settings").getString();

	Ref< PropertyGroup > settings = loadProperties(settingsFile);
	if (!settings)
	{
		traktor::log::error << L"Unable to load settings \"" << settingsFile << L"\"" << Endl;
		return 1;
	}

	int32_t agentCount = c_defaultAgentCount;
	if (cmdLine.hasOption('n', L"agents"))
	{
		agentCount = cmdLine.getOption('n', L"agents").getInteger();
		if (agentCount <= 0)
		{
			traktor::log::error << L"Invalid number of agents; must be atleast one" << Endl;
			return 1;
		}
	}

	std::set< std::wstring > modules = settings->getProperty< PropertyStringSet >(L"Editor.Modules");
	for (std::set< std::wstring >::const_iterator i = modules.begin(); i != modules.end(); ++i)
	{
		Library library;
		if (!library.open(*i))
		{
			traktor::log::error << L"Unable to load module \"" << *i << L"\"" << Endl;
			return 2;
		}
		library.detach();
	}

	net::Network::initialize();

	Ref< net::TcpSocket > serverSocket = new net::TcpSocket();
	if (!serverSocket->bind(net::SocketAddressIPv4(c_defaultAgentPort)))
	{
		traktor::log::error << L"Unable to bind server socket to port " << c_defaultAgentPort << Endl;
		return 1;
	}

	if (!serverSocket->listen())
	{
		traktor::log::error << L"Unable to listen on server socket" << Endl;
		return 2;
	}

	Ref< net::DiscoveryManager > discoveryManager = new net::DiscoveryManager();
	if (!discoveryManager->create(false))
	{
		traktor::log::error << L"Unable to create discovery manager" << Endl;
		return 3;
	}

	net::SocketAddressIPv4::Interface itf;
	if (!net::SocketAddressIPv4::getBestInterface(itf))
	{
		traktor::log::error << L"Unable to get interfaces" << Endl;
		return 4;
	}

	Ref< PropertyGroup > properties = new PropertyGroup();
	properties->setProperty< PropertyString >(L"Description", OS::getInstance().getComputerName());
	properties->setProperty< PropertyString >(L"Host", itf.addr->getHostName());
	properties->setProperty< PropertyInteger >(L"Port", c_defaultAgentPort);
	properties->setProperty< PropertyInteger >(L"Agents", agentCount);

	std::set< std::wstring > types;

	/*
	std::vector< const TypeInfo* > pipelineTypes;
	type_of< editor::IPipeline >().findAllOf(pipelineTypes, false);
	for (std::vector< const TypeInfo* >::const_iterator i = pipelineTypes.begin(); i != pipelineTypes.end(); ++i)
		types.insert((*i)->getName());

	properties->setProperty< PropertyStringSet >(L"PipelineTypes", types);
	*/

	discoveryManager->addService(new net::NetworkService(L"Pipeline/Agent", properties));

	traktor::log::info << L"Discoverable as \"Pipeline/Agent\", host \"" << itf.addr->getHostName() << L"\"" << Endl;
	traktor::log::info << L"Waiting for client(s)..." << Endl;

	std::map< std::wstring, Ref< db::Database > > databases;
	std::list< Thread* > clientThreads;
	for (;;)
	{
		if (serverSocket->select(true, false, false, 1000) <= 0)
		{
			for (std::list< Thread* >::iterator i = clientThreads.begin(); i != clientThreads.end(); )
			{
				if ((*i)->wait(0))
				{
					traktor::log::info << L"Client thread destroyed" << Endl;
					ThreadManager::getInstance().destroy(*i);
					i = clientThreads.erase(i);
				}
				else
					++i;
			}
			continue;
		}

		Ref< net::TcpSocket > clientSocket = serverSocket->accept();
		if (!clientSocket)
			continue;

		Ref< net::BidirectionalObjectTransport > clientTransport = new net::BidirectionalObjectTransport(clientSocket);

		Ref< editor::AgentConnect > agentConnect;
		if (clientTransport->recv< editor::AgentConnect >(10000, agentConnect) != 1)
		{
			log::error << L"Agent build error; no AgentConnect message received" << Endl;
			continue;
		}

		log::info << L"Agent connect message received" << Endl;
		log::info << L"\tHost \"" << agentConnect->getHost() << L"\"" << Endl;
		log::info << L"\tDatabase port " << agentConnect->getDatabasePort() << Endl;
		log::info << L"\tStream server port " << agentConnect->getStreamServerPort() << Endl;

		// Setup a remote database connection to source and output databases.
		Ref< db::Database > sourceDatabase = databases[agentConnect->getSessionId().format() + L"|Source"];
		if (!sourceDatabase)
		{
			db::ConnectionString cs;

			cs.set(L"database", agentConnect->getSessionId().format() + L"|Source");
			cs.set(L"host", agentConnect->getHost() + L":" + toString(agentConnect->getDatabasePort()));
			cs.set(L"provider", L"traktor.db.RemoteDatabase");

			sourceDatabase = new db::Database();
			if (!sourceDatabase->open(cs))
			{
				log::error << L"Agent build error; unable to open source database" << Endl;
				continue;
			}

			databases[agentConnect->getSessionId().format() + L"|Source"] = sourceDatabase;
		}

		Ref< db::Database > outputDatabase = databases[agentConnect->getSessionId().format() + L"|Output"];
		if (!outputDatabase)
		{
			db::ConnectionString cs;

			cs.set(L"database", agentConnect->getSessionId().format() + L"|Output");
			cs.set(L"host", agentConnect->getHost() + L":" + toString(agentConnect->getDatabasePort()));
			cs.set(L"provider", L"traktor.db.RemoteDatabase");

			outputDatabase = new db::Database();
			if (!outputDatabase->open(cs))
			{
				log::error << L"Agent build error; unable to open output database" << Endl;
				continue;
			}

			databases[agentConnect->getSessionId().format() + L"|Output"] = outputDatabase;
		}

		traktor::log::info << L"Connection established; spawning thread..." << Endl;

		Thread* clientThread = ThreadManager::getInstance().create(
			makeStaticFunctor
			<
				Ref< editor::AgentConnect >,
				Ref< net::BidirectionalObjectTransport >,
				Ref< db::Database >,
				Ref< db::Database >
			>
			(
				&threadProcessClient,
				agentConnect,
				clientTransport,
				sourceDatabase,
				outputDatabase
			),
			L"Client thread"
		);
		if (!clientThread)
		{
			traktor::log::error << L"Unable to create client thread" << Endl;
			continue;
		}

		clientThread->start();
		clientThreads.push_back(clientThread);
	}

	return 0;
}
