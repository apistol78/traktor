#include "Compress/Lzo/InflateStreamLzo.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Reader.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringArray.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Discovery/NetworkService.h"
#include "Remote/Server/App/Server.h"

namespace traktor
{
    namespace remote
    {
        namespace
        {

const uint16_t c_listenPort = 50001;
const uint8_t c_msgDeploy = 1;
const uint8_t c_msgLaunchProcess = 2;
const uint8_t c_errNone = 0;
const uint8_t c_errIoFailed = 1;
const uint8_t c_errLaunchFailed = 2;
const uint8_t c_errUnknown = 255;

        }

T_IMPLEMENT_RTTI_CLASS(L"traktor.remote.Server", Server, Object)

bool Server::create(const std::wstring& scratchPath, const std::wstring& keyword, bool verbose)
{
	m_scratchPath = scratchPath;
    m_keyword = keyword;

    // Ensure our scratch path folder exist.
	if (!FileSystem::getInstance().makeAllDirectories(m_scratchPath))
	{
		log::error << L"Unable to create scratch directory \"" << m_scratchPath << L"\"." << Endl;
		return false;
	}

	// Create server socket.
	m_serverSocket = new net::TcpSocket();
	if (!m_serverSocket->bind(net::SocketAddressIPv4(c_listenPort)))
	{
		log::error << L"Unable to bind server socket to port " << c_listenPort << Endl;
		return false;
	}

	if (!m_serverSocket->listen())
	{
		log::error << L"Unable to listen on server socket." << Endl;
		return false;
	}

	// Create discovery manager.
	int32_t mode = net::MdPublishServices;
	if (verbose)
		mode |= net::MdVerbose;

	m_discoveryManager = new net::DiscoveryManager();
	if (!m_discoveryManager->create(mode))
	{
		log::error << L"Unable to create discovery manager" << Endl;
		return false;
	}

    return true;
}

void Server::destroy()
{
}

bool Server::update()
{
    // Keep published interface up-to-date.
    net::SocketAddressIPv4::Interface itf;
    if (!net::SocketAddressIPv4::getBestInterface(itf))
    {
        log::error << L"Unable to get interfaces." << Endl;
        return false;
    }

    if (itf.addr->getHostName() != m_hostName)
    {
        log::info << L"Discoverable as \"RemoteTools/Server\", host \"" << itf.addr->getHostName() << L"\"" << Endl;

	    std::vector< std::wstring > platforms;
#if defined(_WIN32)
        platforms.push_back(L"Android");
        platforms.push_back(L"Emscripten");
        platforms.push_back(L"PS3");
        platforms.push_back(L"PS4");
        platforms.push_back(L"Win64");
#elif defined(__APPLE__)
        platforms.push_back(L"Android");
        platforms.push_back(L"Emscripten");
        platforms.push_back(L"iOS");
        platforms.push_back(L"OSX");
#elif defined(__LINUX__)
        platforms.push_back(L"Android");
        platforms.push_back(L"Emscripten");
        platforms.push_back(L"Linux");
#elif defined(__RPI__)
    	platforms.push_back(L"RaspberryPI");
#endif

        Ref< PropertyGroup > properties = new PropertyGroup();
        properties->setProperty< PropertyString >(L"Description", OS::getInstance().getComputerName());
        properties->setProperty< PropertyString >(L"Host", itf.addr->getHostName());
        properties->setProperty< PropertyInteger >(L"RemotePort", c_listenPort);
        properties->setProperty< PropertyStringArray >(L"Platforms", platforms);

        if (!m_keyword.empty())
            properties->setProperty< PropertyString >(L"Keyword", m_keyword);

        m_discoveryManager->removeAllServices();
        m_discoveryManager->addService(new net::NetworkService(L"RemoteTools/Server", properties));

        m_hostName = itf.addr->getHostName();
    }

    if (m_serverSocket->select(true, false, false, 1000) <= 0)
        return true;

    Ref< net::TcpSocket > clientSocket = m_serverSocket->accept();
    if (!clientSocket)
        return true;    

    processClient(clientSocket);
    return true;
}

uint8_t Server::handleDeploy(net::TcpSocket* clientSocket)
{
	net::SocketStream clientStream(clientSocket, true, true, 5000);
	Reader reader(&clientStream);
	Writer writer(&clientStream);
	std::wstring user;
	std::wstring pathName;
	uint32_t size;
	uint32_t hash;

	reader >> user;
	reader >> pathName;
	reader >> size;
	reader >> hash;

	log::info << L"Receiving file \"" << pathName << L"\", " << size << L" byte(s)" << Endl;

	Path path(m_scratchPath + L"/" + user + L"/" + pathName);
	bool outOfSync = true;

	auto i = m_fileHashes.find(path.getPathName());
	if (i != m_fileHashes.end())
	{
		// File has already been hashed once; check only against previous hash.
		if (i->second == hash)
		{
			// Hashes match; finally ensure file still exist, could have been manually removed.
			if (FileSystem::getInstance().exist(path))
			{
				log::info << L"File up-to-date; skipping (1)" << Endl;
				outOfSync = false;
			}
		}
	}
	else
	{
		// File hasn't been hashed; calculate hash from file.
		Ref< traktor::IStream > fileStream = FileSystem::getInstance().open(path, File::FmRead);
		if (fileStream)
		{
			Adler32 adler;
			adler.begin();

			uint8_t buffer[4096];
			int32_t nread;
			while ((nread = fileStream->read(buffer, sizeof(buffer))) > 0)
				adler.feed(buffer, nread);

			adler.end();

			fileStream->close();
			fileStream = 0;

			if (adler.get() == hash)
			{
				log::info << L"File up-to-date; skipping (2)" << Endl;
				outOfSync = false;
			}
		}
	}

	if (outOfSync)
	{
		writer << uint8_t(1);

		FileSystem::getInstance().makeAllDirectories(path.getPathOnly());

		Ref< traktor::IStream > fileStream = FileSystem::getInstance().open(path, File::FmWrite);
		if (!fileStream)
		{
			log::error << L"Unable to create file \"" << pathName << L"\"" << Endl;
			return c_errIoFailed;
		}

		compress::InflateStreamLzo inflateStream(&clientStream);
		if (!StreamCopy(fileStream, &inflateStream).execute(size))
		{
			log::error << L"Unable to receive file \"" << pathName << L"\"" << Endl;
			return c_errIoFailed;
		}

		log::info << L"File \"" << pathName << L"\" received successfully" << Endl;

		fileStream->close();
		fileStream = nullptr;
	}
	else
		writer << uint8_t(0);

	m_fileHashes[path.getPathName()] = hash;

	return c_errNone;
}

uint8_t Server::handleLaunchProcess(net::TcpSocket* clientSocket)
{
	net::SocketStream clientStream(clientSocket, true, true, 5000);
	Reader reader(&clientStream);
	Writer writer(&clientStream);
	std::wstring user;
	std::wstring pathName;
	std::wstring arguments;
	bool wait;
	int32_t exitCode;

	reader >> user;
	reader >> pathName;
	reader >> arguments;
	reader >> wait;

	log::info << L"Launching \"" << pathName << L"\"" << Endl;
	log::info << L"\targuments \"" << arguments << L"\"" << Endl;

	Path path(m_scratchPath + L"/" + user + L"/" + pathName);
	Ref< IProcess > process = OS::getInstance().execute(L"\"" + path.getPathName() + L"\" " + arguments, m_scratchPath + L"/" + user, 0, false, false, false);
	if (!process)
	{
		log::error << L"Unable to launch process \"" << pathName << L"\"" << Endl;
		writer << uint8_t(c_errLaunchFailed);
		return c_errLaunchFailed;
	}

	if (wait)
	{
		process->wait();
		exitCode = process->exitCode();
	}

	writer << uint8_t(c_errNone);
	return c_errNone;
}

void Server::processClient(net::TcpSocket* clientSocket)
{
	uint8_t msg;
	uint8_t ret;

	if (clientSocket->select(true, false, false, 5000) <= 0)
	{
		log::info << L"Client terminated unexpectedly (1)." << Endl;
		return;
	}

	int32_t md = clientSocket->recv();
	if (md < 0)
	{
		log::info << L"Client terminated unexpectedly (2)." << Endl;
		return;
	}

	msg = uint8_t(md);
	switch (msg)
	{
	case c_msgDeploy:
		ret = handleDeploy(clientSocket);
		break;

	case c_msgLaunchProcess:
		ret = handleLaunchProcess(clientSocket);
		break;

	default:
		log::error << L"Invalid message ID from client; " << int32_t(msg) << Endl;
		ret = c_errUnknown;
		break;
	}

	clientSocket->close();
	clientSocket = nullptr;
}

    }
}