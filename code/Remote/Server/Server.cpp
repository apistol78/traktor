/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Compress/Lzf/DeflateStreamLzf.h"
#include "Compress/Lzf/InflateStreamLzf.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Reader.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringArray.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Core/Timer/Timer.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Discovery/NetworkService.h"
#include "Remote/Server/Server.h"

namespace traktor::remote
{
    namespace
    {

const uint8_t c_msgDeploy = 1;
const uint8_t c_msgLaunchProcess = 2;
const uint8_t c_msgFetch = 3;
const uint8_t c_errNone = 0;
const uint8_t c_errIoFailed = 1;
const uint8_t c_errLaunchFailed = 2;
const uint8_t c_errUnknown = 255;

       }

T_IMPLEMENT_RTTI_CLASS(L"traktor.remote.Server", Server, Object)

bool Server::create(const std::wstring& scratchPath, const std::wstring& keyword, int32_t listenPort, bool verbose)
{
	m_scratchPath = scratchPath;
    m_keyword = keyword;
	m_verbose = verbose;

    // Ensure our scratch path folder exist.
	if (!FileSystem::getInstance().makeAllDirectories(m_scratchPath))
	{
		log::error << L"Unable to create scratch directory \"" << m_scratchPath << L"\"." << Endl;
		return false;
	}

	// Create server socket.
	m_serverSocket = new net::TcpSocket();
	if (!m_serverSocket->bind(net::SocketAddressIPv4(listenPort)))
	{
		log::error << L"Unable to bind server socket." << Endl;
		return false;
	}

	if (!m_serverSocket->listen())
	{
		log::error << L"Unable to listen on server socket." << Endl;
		return false;
	}

	if ((m_listenPort = listenPort) <= 0)
		m_listenPort = dynamic_type_cast< net::SocketAddressIPv4* >(m_serverSocket->getLocalAddress())->getPort();

	// Create discovery manager.
	m_discoveryManager = new net::DiscoveryManager();
	if (!m_discoveryManager->create(net::MdPublishServices))
	{
		log::error << L"Unable to create discovery manager." << Endl;
		return false;
	}

    return true;
}

void Server::destroy()
{
	safeDestroy(m_discoveryManager);
	safeClose(m_serverSocket);
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
		if (m_verbose)
        	log::info << L"Discoverable as \"RemoteTools/Server\", host \"" << itf.addr->getHostName() << L"\"" << Endl;

	    AlignedVector< std::wstring > platforms;
#if defined(_WIN32)
        platforms.push_back(L"Android");
        platforms.push_back(L"Win64");
#elif defined(__APPLE__)
        platforms.push_back(L"Android");
        platforms.push_back(L"iOS");
        platforms.push_back(L"OSX");
#elif defined(__LINUX__)
        platforms.push_back(L"Android");
        platforms.push_back(L"Linux");
#elif defined(__RPI__)
    	platforms.push_back(L"Raspberry PI");
#endif

        Ref< PropertyGroup > properties = new PropertyGroup();
        properties->setProperty< PropertyString >(L"Description", OS::getInstance().getComputerName());
        properties->setProperty< PropertyString >(L"Host", itf.addr->getHostName() + L":" + toString(m_listenPort));
        properties->setProperty< PropertyStringArray >(L"Platforms", platforms);

        if (!m_keyword.empty())
            properties->setProperty< PropertyString >(L"Keyword", m_keyword);

        m_discoveryManager->removeAllServices();
        m_discoveryManager->addService(new net::NetworkService(L"RemoteTools/Server", properties));

        m_hostName = itf.addr->getHostName();
    }

    if (m_serverSocket->select(true, false, false, 500) <= 0)
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

	log::info << L"Receiving file \"" << pathName << L"\", " << size << L" byte(s)." << Endl;

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
				if (m_verbose)
					log::info << L"File up-to-date; skipping (1)." << Endl;
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
			int64_t nread;
			while ((nread = fileStream->read(buffer, sizeof(buffer))) > 0)
				adler.feed(buffer, nread);

			adler.end();

			fileStream->close();
			fileStream = nullptr;

			if (adler.get() == hash)
			{
				if (m_verbose)
					log::info << L"File up-to-date; skipping (2)." << Endl;
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
			log::error << L"Unable to create file \"" << pathName << L"\"." << Endl;
			return c_errIoFailed;
		}

		compress::InflateStreamLzf inflateStream(&clientStream);
		if (!StreamCopy(fileStream, &inflateStream).execute(size))
		{
			log::error << L"Unable to receive file \"" << pathName << L"\"." << Endl;
			return c_errIoFailed;
		}

		if (m_verbose)
			log::info << L"File \"" << pathName << L"\" received successfully." << Endl;

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

	log::info << L"Launch \"" << pathName << L"\"." << Endl;

	if (m_verbose)
	{
		log::info << L"\targuments \"" << arguments << L"\"." << Endl;
		log::info << L"\twait \"" << (wait ? L"yes" : L"no") << L"\"." << Endl;
	}

	Path path(m_scratchPath + L"/" + user + L"/" + pathName);
	Ref< IProcess > process = OS::getInstance().execute(
		L"\"" + path.getPathNameOS() + L"\" " + arguments,
		m_scratchPath + L"/" + user,
		nullptr,
		OS::EfNone
	);
	if (!process)
	{
		log::error << L"Unable to launch process \"" << path.getPathNameOS() << L"\"." << Endl;
		writer << uint8_t(c_errLaunchFailed);
		return c_errLaunchFailed;
	}

	if (wait)
	{
		Timer timer;

		if (m_verbose)
			log::info << L"Waiting for process to terminate..." << Endl;

		process->wait();
		exitCode = process->exitCode();

		if (m_verbose)
			log::info << L"Process terminated after " << (int32_t)timer.getElapsedTime() << L" seconds, exit code = " << exitCode << Endl;
	}

	writer << uint8_t(c_errNone);
	return c_errNone;
}

uint8_t Server::handleFetch(net::TcpSocket* clientSocket)
{
	net::SocketStream clientStream(clientSocket, true, true, 5000);
	Reader reader(&clientStream);
	Writer writer(&clientStream);
	std::wstring user;
	std::wstring pathName;

	reader >> user;
	reader >> pathName;

	Path path(m_scratchPath + L"/" + user + L"/" + pathName);

	Ref< File > file = FileSystem::getInstance().get(path);
	if (file)
	{
		log::info << L"Fetch file \"" << pathName << L"\", " << file->getSize() << L"byte(s) ." << Endl;

		Ref< traktor::IStream > fileStream = FileSystem::getInstance().open(path, File::FmRead);
		if (fileStream)
		{
			writer << (int64_t)file->getSize();

			compress::DeflateStreamLzf deflateStream(&clientStream);
			if (!StreamCopy(&deflateStream, fileStream).execute(file->getSize()))
			{
				log::error << L"Unable to serve file \"" << pathName << L"\"; unable to transmit." << Endl;
				return c_errIoFailed;
			}

			fileStream->close();
			fileStream = nullptr;
		}
		else
			writer << (int64_t)-2;
	}
	else
	{
		log::info << L"Fetch file \"" << path.getPathName() << L"\", no such file." << Endl;
		writer << (int64_t)-1;
	}

	return c_errNone;
}

void Server::processClient(net::TcpSocket* clientSocket)
{
	uint8_t msg;
	uint8_t ret;

	if (clientSocket->select(true, false, false, 5000) <= 0)
	{
		log::warning << L"Client terminated unexpectedly (1)." << Endl;
		return;
	}

	int32_t md = clientSocket->recv();
	if (md < 0)
	{
		log::warning << L"Client terminated unexpectedly (2)." << Endl;
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

	case c_msgFetch:
		ret = handleFetch(clientSocket);
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
