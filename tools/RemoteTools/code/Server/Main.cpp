#include <list>
#include <Core/Io/FileSystem.h>
#include <Core/Io/Reader.h>
#include <Core/Io/StreamCopy.h>
#include <Core/Io/Writer.h>
#include <Core/Log/Log.h>
#include <Core/Misc/Adler32.h>
#include <Core/System/IProcess.h>
#include <Core/System/OS.h>
#include <Core/Thread/Thread.h>
#include <Core/Thread/ThreadManager.h>
#include <Net/Network.h>
#include <Net/SocketAddressIPv4.h>
#include <Net/SocketStream.h>
#include <Net/TcpSocket.h>

using namespace traktor;

namespace
{

const uint16_t c_listenPort = 50001;
const uint8_t c_msgDeploy = 1;
const uint8_t c_msgLaunchProcess = 2;
const uint8_t c_errNone = 0;
const uint8_t c_errIoFailed = 1;
const uint8_t c_errLaunchFailed = 2;
const uint8_t c_errUnknown = 255;

uint8_t handleDeploy(net::Socket* clientSocket)
{
	net::SocketStream clientStream(clientSocket, true, true, 1000);
	Reader reader(&clientStream);
	Writer writer(&clientStream);
	std::wstring pathName;
	uint32_t size;
	uint32_t hash;

	reader >> pathName;
	reader >> size;
	reader >> hash;

	traktor::log::info << L"Receiving file \"" << pathName << L"\", " << size << L" byte(s)" << Endl;

	bool outOfSync = true;

	Ref< traktor::IStream > fileStream = FileSystem::getInstance().open(pathName, File::FmRead);
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
			traktor::log::info << L"File up-to-date; skipping" << Endl;
			outOfSync = false;
		}
	}

	if (outOfSync)
	{
		FileSystem::getInstance().makeAllDirectories(Path(pathName).getPathOnly());

		Ref< traktor::IStream > fileStream = FileSystem::getInstance().open(pathName, File::FmWrite);
		if (!fileStream)
		{
			traktor::log::error << L"Unable to create file \"" << pathName << L"\"" << Endl;
			return c_errIoFailed;
		}

		writer << uint8_t(1);

		if (!StreamCopy(fileStream, &clientStream).execute(size))
		{
			traktor::log::error << L"Unable to receive file \"" << pathName << L"\"" << Endl;
			return c_errIoFailed;
		}

		traktor::log::info << L"File \"" << pathName << L"\" received successfully" << Endl;

		fileStream->close();
		fileStream = 0;
	}
	else
		writer << uint8_t(0);

	return c_errNone;
}

uint8_t handleLaunchProcess(net::Socket* clientSocket)
{
	net::SocketStream clientStream(clientSocket, true, true, 1000);
	Reader reader(&clientStream);
	std::wstring pathName;
	std::wstring arguments;
	bool wait;
	int32_t exitCode;

	reader >> pathName;
	reader >> arguments;
	reader >> wait;

	traktor::log::info << L"Launching \"" << pathName << L"\"" << Endl;
	traktor::log::info << L"\targuments \"" << arguments << L"\"" << Endl;

	Ref< IProcess > process = OS::getInstance().execute(pathName, arguments, L".", 0, false, false, false);
	if (!process)
	{
		traktor::log::error << L"Unable to launch process \"" << pathName << L"\"" << Endl;
		return c_errLaunchFailed;
	}

	if (wait)
	{
		process->wait();
		exitCode = process->exitCode();
	}

	return c_errNone;
}

void threadProcessClient(Ref< net::TcpSocket > clientSocket)
{
	uint8_t msg;
	uint8_t ret;

	while (!ThreadManager::getInstance().getCurrentThread()->stopped())
	{
		int32_t res = clientSocket->select(true, false, false, 100);
		if (res < 0)
		{
			traktor::log::info << L"Client terminated (1)" << Endl;
			break;
		}

		if (res == 0)
			continue;

		int32_t md = clientSocket->recv();
		if (md < 0)
		{
			traktor::log::info << L"Client terminated (2)" << Endl;
			break;
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
			traktor::log::error << L"Invalid message ID from client; " << int32_t(msg) << Endl;
			ret = c_errUnknown;
			break;
		}

		clientSocket->send(ret);

		if (ret != c_errNone)
			break;
	}

	clientSocket->close();
	clientSocket = 0;
}

}

int main()
{
	net::Network::initialize();

	Ref< net::TcpSocket > serverSocket = new net::TcpSocket();
	if (!serverSocket->bind(net::SocketAddressIPv4(c_listenPort)))
	{
		traktor::log::error << L"Unable to bind server socket to port " << c_listenPort << Endl;
		return 1;
	}

	if (!serverSocket->listen())
	{
		traktor::log::error << L"Unable to listen on server socket" << Endl;
		return 2;
	}

	traktor::log::info << L"Waiting for client(s)..." << Endl;

	std::list< Thread* > clientThreads;
	for (;;)
	{
		if (serverSocket->select(true, false, false, 100) <= 0)
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

		traktor::log::info << L"Client connected; spawning thread..." << Endl;

		Thread* clientThread = ThreadManager::getInstance().create(
			makeStaticFunctor(&threadProcessClient, clientSocket),
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

	net::Network::finalize();
	return 0;
}
