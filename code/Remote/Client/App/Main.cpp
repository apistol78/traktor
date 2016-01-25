#include "Compress/Lzo/DeflateStreamLzo.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Reader.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/String.h"
#include "Core/System/OS.h"
#include "Net/Network.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"

using namespace traktor;

namespace
{

const uint16_t c_serverPort = 50001;
const uint8_t c_msgDeploy = 1;
const uint8_t c_msgLaunchProcess = 2;
const uint8_t c_errNone = 0;
const uint8_t c_errIoFailed = 1;
const uint8_t c_errLaunchFailed = 2;
const uint8_t c_errUnknown = 255;

}

int launch(const CommandLine& cmdLine)
{
	std::wstring host = cmdLine.getString(1);
	std::wstring application = cmdLine.getString(2);
	bool wait = cmdLine.hasOption('w', L"wait");
	bool base = cmdLine.hasOption('t', L"target-base");
	Path targetBase = base ? cmdLine.getOption('t', L"target-base").getString() : L"";

	if (!targetBase.isRelative())
	{
		log::error << L"Target base must be a relative path." << Endl;
		return 1;
	}

	std::wstring arguments;
	for (int32_t i = 3; i < cmdLine.getCount(); ++i)
	{
		if (!arguments.empty())
			arguments += L" ";

		std::wstring argument = cmdLine.getString(i);
		if (startsWith< std::wstring >(argument, L"\\-"))
			argument = argument.substr(1);

		arguments += argument;
	}

	Ref< net::TcpSocket > clientSocket = new net::TcpSocket();
	if (!clientSocket->connect(net::SocketAddressIPv4(host, c_serverPort)))
	{
		log::error << L"Unable to connect to \"" << host << L"\"." << Endl;
		return 1;
	}

	log::info << L"Successfully connected to \"" << host << "\"." << Endl;

	net::SocketStream clientStream(clientSocket, true, true, 5000);
	Writer writer(&clientStream);
	Reader reader(&clientStream);
	uint8_t ret;

	writer << c_msgLaunchProcess;

	if (!targetBase.getPathName().empty())
		writer << (OS::getInstance().getCurrentUser() + L"/" + targetBase.getPathName());
	else
		writer << OS::getInstance().getCurrentUser();

	writer << application;
	writer << arguments;
	writer << wait;

	reader >> ret;
	if (ret != c_errNone)
		log::error << L"Unable to launch \"" << application << L"\"; server error " << int32_t(ret) << L"." << Endl;

	clientSocket->close();
	clientSocket = 0;
	return 0;
}

bool deployFile(const net::SocketAddressIPv4& addr, const Path& sourceFile, const Path& targetBase)
{
	Ref< net::TcpSocket > clientSocket = new net::TcpSocket();
	if (!clientSocket->connect(addr))
	{
		log::info << L"Unable to connect to \"" << addr.getHostName() << L"\"." << Endl;
		return false;
	}

	net::SocketStream clientStream(clientSocket, true, true, 5000);

	log::info << L"Deploying file \"" << sourceFile.getFileName() << L"\"..." << Endl;

	Path targetFile;
	Path sourceFileA = FileSystem::getInstance().getAbsolutePath(sourceFile).normalized();
	if (!FileSystem::getInstance().getRelativePath(
		sourceFileA,
		FileSystem::getInstance().getAbsolutePath(L"").normalized(),
		targetFile
	))
	{
		log::error << L"Unable to resolve relative path of file \"" << sourceFile.getPathName() << L"\"" << Endl;
		return false;
	}

	if (!targetBase.getPathName().empty())
		targetFile = targetBase + targetFile;

	log::info << L"\ttarget \"" << targetFile.getPathName() << L"\"." << Endl;

	Ref< File > file = FileSystem::getInstance().get(sourceFile);
	if (!file)
	{
		log::error << L"Unable to open file \"" << sourceFile.getPathName() << L"\"; no such file." << Endl;
		return false;
	}

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
	int32_t nread;
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
		compress::DeflateStreamLzo deflateStream(&clientStream);
		if (!StreamCopy(&deflateStream, fileStream).execute(int32_t(file->getSize())))
		{
			log::error << L"Unable to deploy file \"" << sourceFile.getPathName() << L"\"; unable to transmit." << Endl;
			return false;
		}
	}
	else
		log::info << L"File already up-to-date; skipped." << Endl;

	fileStream->close();
	fileStream = 0;

	log::info << L"File deployed successfully." << Endl;
	return true;
}

bool deployFiles(const net::SocketAddressIPv4& addr, const Path& sourcePath, const Path& targetBase, bool recursive)
{
	RefArray< File > files;

	FileSystem::getInstance().find(sourcePath, files);
	log::info << L"Found " << int32_t(files.size()) << L" file(s) matching \"" << sourcePath.getPathName() << L"\"." << Endl;

	for (RefArray< File >::iterator i = files.begin(); i != files.end(); ++i)
	{
		Path sourceFile = (*i)->getPath();
		if ((*i)->isDirectory())
		{
			if (recursive)
			{
				if (sourceFile.getFileName() != L"." && sourceFile.getFileName() != L"..")
				{
					log::info << L"Enter directory \"" << sourceFile.getPathName() << L"\"" << Endl;
					if (!deployFiles(addr, sourceFile.getPathName() + L"/" + sourcePath.getFileName(), targetBase, true))
						return false;
					log::info << L"Leaving directory \"" << sourceFile.getPathName() << L"\"" << Endl;
				}
			}
			else
				log::info << L"Directory \"" << sourceFile.getPathName() << L"\" skipped" << Endl;
		}
		else
		{
			if (!deployFile(addr, sourceFile, targetBase))
				return false;
		}
	}
	return true;
}

int deploy(const CommandLine& cmdLine)
{
	std::wstring host = cmdLine.getString(1);
	bool recursive = cmdLine.hasOption('r', L"recursive");
	bool base = cmdLine.hasOption('t', L"target-base");
	Path targetBase = base ? cmdLine.getOption('t', L"target-base").getString() : L"";

	if (!targetBase.isRelative())
	{
		log::error << L"Target base must be a relative path." << Endl;
		return 1;
	}

	net::SocketAddressIPv4 addr(host, c_serverPort);

	int32_t ret = 0;
	for (int i = 2; i < cmdLine.getCount(); ++i)
	{
		Path sourcePath = cmdLine.getString(i);
		if (!deployFiles(addr, sourcePath, targetBase, recursive))
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
		log::info << L"Usage:" << Endl;
		log::info << Endl;
		log::info << L"  Traktor.Remote.Client.App deploy <host> (options) <application> (arguments...)" << Endl;
		log::info << L"    -w, -wait	                       Wait until application terminates." << Endl;
		log::info << L"    -t=<base>, -target-base=<base>  Target base path." << Endl;
		log::info << Endl;
		log::info << L"  Traktor.Remote.Client.App launch <host> (options) <files>" << Endl;
		log::info << L"    -r, -recursive                  Recursive deploy files in directories." << Endl;
		log::info << L"    -t=<base>, -target-base=<base>  Target base path." << Endl;
		return 0;
	}

	log::info << L"Traktor.Remote.Client.App; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;

	net::Network::initialize();

	if (compareIgnoreCase< std::wstring >(cmdLine.getString(0), L"deploy") == 0)
		ret = deploy(cmdLine);
	else if (compareIgnoreCase< std::wstring >(cmdLine.getString(0), L"launch") == 0)
		ret = launch(cmdLine);
	else
		log::error << L"Unknown operation \"" << cmdLine.getString(0) << L"\"." << Endl;

	net::Network::finalize();
	return ret;
}
