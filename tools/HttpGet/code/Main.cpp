/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Net/Network.h>
#include <Net/Url.h>
#include <Net/UrlConnection.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/IStream.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Log/Log.h>

using namespace traktor;

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	std::wstring fileName;

	if (cmdLine.getCount() < 1)
	{
		log::info << L"Usage: HttpGet (option(s)) [url] (file)" << Endl;
		log::info << L"   -v, -verbose   Verbose log" << Endl;
		return 0;
	}

	net::Network::initialize();

	net::Url url(cmdLine.getString(0));
	if (!url.valid())
	{
		log::error << L"Invalid URL" << Endl;
		return 1;
	}

	if (cmdLine.hasOption(L'v', L"verbose"))
		log::info << L"Connecting to " << url.getString() << L"..." << Endl;

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

	Ref< traktor::IStream > stream = connection->getStream();
	T_ASSERT (stream);

	if (cmdLine.hasOption(L'v', L"verbose"))
		log::info << L"Creating target file " << fileName << L"..." << Endl;

	Ref< traktor::IStream > file = FileSystem::getInstance().open(fileName, File::FmWrite);
	if (!file)
	{
		log::error << L"Unable to create file \"" << fileName << L"\"" << Endl;
		return 3;
	}

	if (cmdLine.hasOption(L'v', L"verbose"))
		log::info << L"Downloading..." << Endl;

	uint32_t total = 0;
	uint8_t buf[4096];

	for (;;)
	{
		int nread = stream->read(buf, sizeof(buf));
		if (nread <= 0)
			break;

		total += file->write(buf, nread);
	}

	if (cmdLine.hasOption(L'v', L"verbose"))
		log::info << L"Received " << total << L" byte(s)" << Endl;

	file->close();
	stream->close();

	net::Network::finalize();
	return 0;
}
