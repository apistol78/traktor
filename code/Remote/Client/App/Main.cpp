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
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/System/OS.h"
#include "Net/Network.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"

using namespace traktor;

namespace
{

const uint8_t c_msgDeploy = 1;
const uint8_t c_msgLaunchProcess = 2;
const uint8_t c_msgFetch = 3;
const uint8_t c_errNone = 0;
const uint8_t c_errIoFailed = 1;
const uint8_t c_errLaunchFailed = 2;
const uint8_t c_errUnknown = 255;

int32_t parseHost(std::wstring& host)
{
	int32_t port = 50001;
	size_t p = host.find(L':');
	if (p != host.npos)
	{
		port = parseString< int32_t >(host.substr(p + 1));
		host = host.substr(0, p);
	}
	return port;
}

}

int launch(const CommandLine& cmdLine)
{
	std::wstring host = cmdLine.getString(1);
	const std::wstring application = cmdLine.getString(2);
	const bool wait = cmdLine.hasOption('w', L"wait");
	const bool base = cmdLine.hasOption('t', L"target-base");
	const bool verbose = cmdLine.hasOption('v', L"verbose");
	const Path targetBase = base ? cmdLine.getOption('t', L"target-base").getString() : L"";

	if (!targetBase.isRelative())
	{
		log::error << L"Target base must be a relative path." << Endl;
		return 1;
	}

	if (verbose && base)
		log::info << L"Using target base \"" << targetBase.getPathName() << L"\"" << Endl;

	std::wstring arguments;
	for (int32_t i = 3; i < cmdLine.getCount(); ++i)
	{
		if (!arguments.empty())
			arguments += L" ";

		std::wstring argument = cmdLine.getString(i);
		if (startsWith(argument, L"\\-"))
			argument = argument.substr(1);

		arguments += argument;
	}

	int32_t port = parseHost(host);

	Ref< net::TcpSocket > clientSocket = new net::TcpSocket();
	if (!clientSocket->connect(net::SocketAddressIPv4(host, port)))
	{
		log::error << L"Unable to connect to \"" << host << L":" << port << L"\"." << Endl;
		return 1;
	}

	if (verbose)
		log::info << L"Successfully connected to \"" << host << L"\"." << Endl;

	clientSocket->setNoDelay(true);

	net::SocketStream clientStream(clientSocket, true, true, 5000);
	Writer writer(&clientStream);
	Reader reader(&clientStream);
	uint8_t ret = 0xff;

	writer << c_msgLaunchProcess;

	if (!targetBase.getPathName().empty())
		writer << (OS::getInstance().getCurrentUser() + L"/" + targetBase.getPathName());
	else
		writer << OS::getInstance().getCurrentUser();

	writer << application;
	writer << arguments;
	writer << wait;

	if (wait)
		clientStream.setTimeout(-1);

	reader >> ret;
	if (ret != c_errNone)
		log::error << L"Unable to launch \"" << application << L"\"; server error " << int32_t(ret) << L"." << Endl;

	clientSocket->close();
	clientSocket = nullptr;
	return 0;
}

bool deployFile(const net::SocketAddressIPv4& addr, const Path& sourceFile, const Path& targetBase, bool verbose)
{
	Ref< net::TcpSocket > clientSocket = new net::TcpSocket();
	if (!clientSocket->connect(addr))
	{
		log::info << L"Unable to connect to \"" << addr.getHostName() << L":" << addr.getPort() << L"\"." << Endl;
		return false;
	}

	clientSocket->setNoDelay(true);

	net::SocketStream clientStream(clientSocket, true, true, 5000);

	if (verbose)
		log::info << L"Deploying file \"" << sourceFile.getFileName() << L"\"..." << Endl;

	Path targetFile;
	Path sourceFileA = FileSystem::getInstance().getAbsolutePath(sourceFile).normalized();
	if (!FileSystem::getInstance().getRelativePath(
		sourceFileA,
		FileSystem::getInstance().getAbsolutePath(L"").normalized(),
		targetFile
	))
	{
		log::error << L"Unable to resolve relative path of file \"" << sourceFile.getPathName() << L"\"." << Endl;
		return false;
	}

	if (!targetBase.getPathName().empty())
		targetFile = targetBase + targetFile;

	if (verbose)
		log::info << L"\ttarget \"" << targetFile.getPathName() << L"\"." << Endl;

	Ref< File > file = FileSystem::getInstance().get(sourceFile);
	if (!file)
	{
		log::error << L"Unable to open file \"" << sourceFile.getPathName() << L"\"; no such file." << Endl;
		return false;
	}

	if (verbose)
		log::info << L"\tsize " << file->getSize() << L" byte(s)." << Endl;

	Ref< traktor::IStream > fileStream = FileSystem::getInstance().open(sourceFile, File::FmRead);
	if (!fileStream)
	{
		log::error << L"Unable to open source file \"" << sourceFile.getPathName() << L"\"." << Endl;
		return false;
	}

	Adler32 adler;
	adler.begin();

	uint8_t buffer[4096];
	int64_t nread;
	while ((nread = fileStream->read(buffer, sizeof(buffer))) > 0)
		adler.feed(buffer, nread);

	adler.end();

	Writer writer(&clientStream);
	Reader reader(&clientStream);
	uint8_t ret;

	writer << c_msgDeploy;
	writer << OS::getInstance().getCurrentUser();
	writer << targetFile.getPathName();
	writer << uint32_t(file->getSize());
	writer << uint32_t(adler.get());

	reader >> ret;

	if (ret == 1)
	{
		fileStream->seek(traktor::IStream::SeekSet, 0);

		clientStream.setTimeout(-1);
		compress::DeflateStreamLzf deflateStream(&clientStream);
		if (!StreamCopy(&deflateStream, fileStream).execute(int32_t(file->getSize())))
		{
			log::error << L"Unable to deploy file \"" << sourceFile.getPathName() << L"\"; unable to transmit." << Endl;
			return false;
		}
	}
	else if (verbose)
		log::info << L"File already up-to-date; skipped." << Endl;

	fileStream->close();
	fileStream = nullptr;

	if (verbose)
		log::info << L"File deployed successfully." << Endl;
	return true;
}

bool deployFiles(const net::SocketAddressIPv4& addr, const Path& sourcePath, const Path& targetBase, bool recursive, bool verbose, int32_t& inoutDeployedFileCount)
{
	RefArray< File > files = FileSystem::getInstance().find(sourcePath);

	if (verbose)
		log::info << L"Found " << int32_t(files.size()) << L" file(s) matching \"" << sourcePath.getPathName() << L"\"." << Endl;

	for (auto file : files)
	{
		Path sourceFile = file->getPath();
		if (file->isDirectory())
		{
			if (recursive)
			{
				if (sourceFile.getFileName() != L"." && sourceFile.getFileName() != L"..")
				{
					if (verbose)
						log::info << L"Enter directory \"" << sourceFile.getPathName() << L"\"." << Endl;
					if (!deployFiles(addr, sourceFile.getPathName() + L"/" + sourcePath.getFileName(), targetBase, true, verbose, inoutDeployedFileCount))
						return false;
					if (verbose)
						log::info << L"Leaving directory \"" << sourceFile.getPathName() << L"\"." << Endl;
				}
			}
			else if (verbose)
				log::info << L"Directory \"" << sourceFile.getPathName() << L"\" skipped." << Endl;
		}
		else
		{
			if (!deployFile(addr, sourceFile, targetBase, verbose))
				return false;
			++inoutDeployedFileCount;
		}
	}
	return true;
}

int deploy(const CommandLine& cmdLine)
{
	std::wstring host = cmdLine.getString(1);
	const bool recursive = cmdLine.hasOption('r', L"recursive");
	const bool base = cmdLine.hasOption('t', L"target-base");
	const bool verbose = cmdLine.hasOption('v', L"verbose");
	const Path targetBase = base ? cmdLine.getOption('t', L"target-base").getString() : L"";

	if (!targetBase.isRelative())
	{
		log::error << L"Target base must be a relative path." << Endl;
		return 1;
	}

	if (cmdLine.getCount() <= 2)
	{
		log::error << L"No files to deploy." << Endl;
		return 2;
	}

	if (verbose && base)
		log::info << L"Using target base \"" << targetBase.getPathName() << L"\"" << Endl;

	int32_t port = parseHost(host);
	net::SocketAddressIPv4 addr(host, port);

	int32_t ret = 0;
	int32_t deployedFileCount = 0;
	for (int i = 2; i < cmdLine.getCount(); ++i)
	{
		const Path sourcePath = cmdLine.getString(i);
		if (!deployFiles(addr, sourcePath, targetBase, recursive, verbose, deployedFileCount))
		{
			ret = 3;
			break;
		}
	}

	if (ret == 0 && deployedFileCount <= 0)
	{
		log::error << L"No files deployed." << Endl;
		return 4;
	}

	return ret;
}

bool fetchFile(const net::SocketAddressIPv4& addr, const Path& fileName, const Path& targetBase, bool verbose)
{
	Ref< net::TcpSocket > clientSocket = new net::TcpSocket();
	if (!clientSocket->connect(addr))
	{
		log::info << L"Unable to connect to \"" << addr.getHostName() << L":" << addr.getPort() << L"\"." << Endl;
		return false;
	}

	clientSocket->setNoDelay(true);

	Path targetFile = fileName;
	if (!targetBase.getPathName().empty())
		targetFile = targetBase + targetFile;

	net::SocketStream clientStream(clientSocket, true, true, 5000);

	Writer writer(&clientStream);
	Reader reader(&clientStream);
	int64_t ret;

	writer << c_msgFetch;
	writer << OS::getInstance().getCurrentUser();
	writer << targetFile.getPathName();

	reader >> ret;

	if (ret >= 0)
	{
		Ref< traktor::IStream > fileStream = FileSystem::getInstance().open(fileName, File::FmWrite);
		if (!fileStream)
		{
			log::error << L"Unable to create file \"" << fileName.getPathName() << L"\"." << Endl;
			return false;
		}

		compress::InflateStreamLzf inflateStream(&clientStream);
		if (!StreamCopy(fileStream, &inflateStream).execute(ret))
		{
			log::error << L"Unable to receive file \"" << fileName.getPathName() << L"\"." << Endl;
			return false;
		}

		if (verbose)
			log::info << L"File \"" << fileName.getPathName() << L"\" fetched successfully." << Endl;

		fileStream->close();
		fileStream = nullptr;
	}
	else
	{
		log::error << L"Unable to fetch \"" << fileName.getPathName() << L"\"." << Endl;
		return false;
	}

	return true;
}

int fetch(const CommandLine& cmdLine)
{
	std::wstring host = cmdLine.getString(1);
	const bool base = cmdLine.hasOption('t', L"target-base");
	const bool verbose = cmdLine.hasOption('v', L"verbose");
	const Path targetBase = base ? cmdLine.getOption('t', L"target-base").getString() : L"";

	if (!targetBase.isRelative())
	{
		log::error << L"Target base must be a relative path." << Endl;
		return 1;
	}

	if (cmdLine.getCount() <= 2)
	{
		log::error << L"No files to fetch." << Endl;
		return 2;
	}

	if (verbose && base)
		log::info << L"Using target base \"" << targetBase.getPathName() << L"\"" << Endl;

	int32_t port = parseHost(host);
	net::SocketAddressIPv4 addr(host, port);

	int32_t ret = 0;
	for (int i = 2; i < cmdLine.getCount(); ++i)
	{
		const Path fileName = cmdLine.getString(i);
		if (!fetchFile(addr, fileName, targetBase, verbose))
		{
			ret = 3;
			break;
		}
	}

	return ret;
}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);
	int32_t ret = 1;

	if (cmdLine.getCount() < 3)
	{
		log::info << L"Traktor.Remote.Client.App; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;
		log::info << L"Usage:" << Endl;
		log::info << Endl;
		log::info << L"  Traktor.Remote.Client.App deploy <host> (options) <application> (arguments...)" << Endl;
		log::info << L"    -w, -wait	                   Wait until application terminates." << Endl;
		log::info << L"    -t=<base>, -target-base=<base>  Target base path." << Endl;
		log::info << L"    -v, -verbose                    Verbose logging." << Endl; 
		log::info << Endl;
		log::info << L"  Traktor.Remote.Client.App launch <host> (options) <files>" << Endl;
		log::info << L"    -r, -recursive                  Recursive deploy files in directories." << Endl;
		log::info << L"    -t=<base>, -target-base=<base>  Target base path." << Endl;
		log::info << L"    -v, -verbose                    Verbose logging." << Endl; 
		log::info << Endl;
		log::info << L"  Traktor.Remote.Client.App fetch <host> (options) <files>" << Endl;
		log::info << L"    -t=<base>, -target-base=<base>  Target base path." << Endl;
		log::info << L"    -v, -verbose                    Verbose logging." << Endl; 
		return 0;
	}

	const bool verbose = cmdLine.hasOption('v', L"verbose");

	if (verbose)
		log::info << L"Traktor.Remote.Client.App; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;

	if (!net::Network::initialize())
	{
		log::error << L"Unable to initialize networking." << Endl;
		return 1;
	}

	if (compareIgnoreCase(cmdLine.getString(0), L"deploy") == 0)
		ret = deploy(cmdLine);
	else if (compareIgnoreCase(cmdLine.getString(0), L"launch") == 0)
		ret = launch(cmdLine);
	else if (compareIgnoreCase(cmdLine.getString(0), L"fetch") == 0)
		ret = fetch(cmdLine);
	else
		log::error << L"Unknown operation \"" << cmdLine.getString(0) << L"\"." << Endl;

	net::Network::finalize();
	return ret;
}
