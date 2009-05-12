#include <Net/Network.h>
#include <Net/Url.h>
#include <Net/UrlConnection.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/Stream.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Log/Log.h>

using namespace traktor;

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	std::wstring fileName;

	if (cmdLine.getCount() < 1)
	{
		log::info << L"Usage: HttpGet [url] (file)" << Endl;
		return 0;
	}

	net::Network::initialize();

	net::Url url(cmdLine.getString(0));
	if (!url.valid())
	{
		log::error << L"Invalid URL" << Endl;
		return 1;
	}

	Ref< net::UrlConnection > connection = net::UrlConnection::open(url);
	if (!connection)
	{
		log::error << L"Unable to establish connection to \"" << url.getString() << L"\"" << Endl;
		return 2;
	}

	if (cmdLine.getCount() > 1)
		fileName = cmdLine.getString(1);
	else
		fileName = connection->getUrl().getFile();

	Ref< Stream > stream = connection->getStream();
	T_ASSERT (stream);

	Ref< Stream > file = FileSystem::getInstance().open(fileName, File::FmWrite);
	if (!file)
	{
		log::error << L"Unable to create file \"" << fileName << L"\"" << Endl;
		return 3;
	}

	uint32_t total = 0;
	uint8_t buf[4096];

	for (;;)
	{
		int avail = stream->available();
		if (avail <= 0)
			break;

		int read = std::min< int >(avail, sizeof(buf));

		int nread = stream->read(buf, read);
		if (nread <= 0)
			break;

		total += file->write(buf, nread);
	}

	log::info << L"Recieved " << total << L" byte(s)" << Endl;

	file->close();
	stream->close();

	net::Network::finalize();
	return 0;
}
