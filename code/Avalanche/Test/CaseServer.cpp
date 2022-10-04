#include <cstring>
#include "Avalanche/Dictionary.h"
#include "Avalanche/Client/Client.h"
#include "Avalanche/Server/Server.h"
#include "Avalanche/Test/CaseServer.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"

namespace traktor::avalanche::test
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.avalanche.test.CaseServer", 0, CaseServer, traktor::test::Case)

void CaseServer::run()
{
	Ref< PropertyGroup > settings = new PropertyGroup();
	settings->setProperty< PropertyInteger >(L"Avalanche.ListenPort", 20001);

	Ref< Server > server = new Server();
	server->create(settings);

	Thread* serverThread = ThreadManager::getInstance().create([&](){
		while (!serverThread->stopped())
			server->update();
	});
	CASE_ASSERT(serverThread != nullptr);
	if (serverThread == nullptr)
		return;

	serverThread->start();


	Ref< Client > client = new Client(net::SocketAddressIPv4(L"localhost", 20001));

	const wchar_t c_blob[] = { L"Hello world!" };

	{
		const Key key = { 1, 2, 3, 4 };
		auto s = client->get(key);
		CASE_ASSERT(s == nullptr);
	}

	{
		const Key key = { 1, 2, 3, 4 };
		auto s = client->put(key);
		int64_t nwritten = s->write(c_blob, sizeof(c_blob));
		s->close();
		CASE_ASSERT(nwritten == sizeof(c_blob));
	}

	{
		const Key key = { 1, 2, 3, 4 };
		auto s = client->get(key);
		CASE_ASSERT(s != nullptr);
		if (s)
		{
			int64_t avail = s->available();
			CASE_ASSERT(avail == sizeof(c_blob));

			if (avail == sizeof(c_blob))
			{
				AlignedVector< uint8_t > blob(avail);
				int64_t read = s->read(blob.ptr(), avail);
				CASE_ASSERT(read == avail);
				CASE_ASSERT(std::memcmp(blob.c_ptr(), c_blob, sizeof(c_blob)) == 0);
			}

			s->close();
		}
	}

	client->destroy();



	ThreadManager::getInstance().getCurrentThread()->sleep(1000);
	CASE_ASSERT(server->getConnectionCount() == 0);

	serverThread->stop();

	server->destroy();
	server = nullptr;
}

}
