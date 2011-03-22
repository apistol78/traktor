#include <Core/Io/FileSystem.h>
#include <Core/Io/Reader.h>
#include <Core/Io/StreamCopy.h>
#include <Core/Io/Writer.h>
#include <Core/Log/Log.h>
#include <Core/Misc/Adler32.h>
#include <Core/Misc/CommandLine.h>
#include <Net/Network.h>
#include <Net/SocketAddressIPv4.h>
#include <Net/SocketStream.h>
#include <Net/TcpSocket.h>

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

bool deployFile(net::Socket* clientSocket, const Path& sourceFile)
{
	net::SocketStream clientStream(clientSocket, true, true, 1000);

	traktor::log::info << L"Deploying file \"" << sourceFile.getFileName() << L"\"..." << Endl;

	Path targetFile;
	Path sourceFileA = FileSystem::getInstance().getAbsolutePath(sourceFile).normalized();
	if (!FileSystem::getInstance().getRelativePath(
		sourceFileA,
		FileSystem::getInstance().getAbsolutePath(L"").normalized(),
		targetFile
	))
	{
		traktor::log::error << L"Unable to resolve relative path of file \"" << sourceFile.getPathName() << L"\"" << Endl;
		return false;
	}

	traktor::log::info << L"\ttarget \"" << targetFile.getPathName() << L"\"" << Endl;

	Ref< File > file = FileSystem::getInstance().get(sourceFile);
	if (!file)
	{
		traktor::log::error << L"Unable to open file \"" << sourceFile.getPathName() << L"\"; no such file" << Endl;
		return false;
	}

	traktor::log::info << L"\tsize " << file->getSize() << L" byte(s)" << Endl;
	
	Ref< traktor::IStream > fileStream = FileSystem::getInstance().open(sourceFile, File::FmRead);
	if (!fileStream)
	{
		traktor::log::error << L"Unable to open source file \"" << sourceFile.getPathName() << L"\"" << Endl;
		return false;
	}

	Adler32 adler;
	adler.begin();

	uint8_t buffer[4096];
	int32_t nread;
	while ((nread = fileStream->read(buffer, sizeof(buffer))) > 0)
		adler.feed(buffer, nread);
	
	adler.end();

	fileStream->seek(traktor::IStream::SeekSet, 0);

	Writer writer(&clientStream);
	Reader reader(&clientStream);
	uint8_t ret;

	writer << c_msgDeploy;
	writer << targetFile.getPathName();
	writer << uint32_t(file->getSize());
	writer << uint32_t(adler.get());

	reader >> ret;

	if (ret == 1)
	{
		if (!StreamCopy(&clientStream, fileStream).execute(int32_t(file->getSize())))
		{
			traktor::log::error << L"Unable to deploy file \"" << sourceFile.getPathName() << L"\"; unable to transmit" << Endl;
			return false;
		}

		traktor::log::info << L"File transmitted successfully" << Endl;
	}
	else
		traktor::log::info << L"File already up-to-date; skipped" << Endl;

	fileStream->close();
	fileStream = 0;

	if (clientSocket->select(true, false, false, 1000) > 0)
		reader >> ret;
	else
		ret = c_errUnknown;

	if (ret != c_errNone)
		traktor::log::error << L"Unable to deploy file \"" << sourceFile.getPathName() << L"\"; server error " << int32_t(ret) << Endl;
	else
		traktor::log::info << L"File deployed successfully" << Endl;

	return ret == c_errNone;
}

bool deployFiles(net::Socket* clientSocket, const Path& sourcePath, bool recursive)
{
	RefArray< File > files;
	FileSystem::getInstance().find(sourcePath, files);
	for (RefArray< File >::iterator i = files.begin(); i != files.end(); ++i)
	{
		Path sourceFile = (*i)->getPath();
		if ((*i)->isDirectory())
		{
			if (recursive)
			{
				if (sourceFile.getPathName() != L"." && sourceFile.getPathName() != L"..")
				{
					if (!deployFiles(clientSocket, sourceFile, true))
						return false;
				}
			}
		}
		else
		{
			if (!deployFile(clientSocket, sourceFile))
				return false;
		}
	}
	return true;
}

}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);
	
	if (cmdLine.getCount() < 2)
	{
		traktor::log::info << L"Usage: RemoteDeploy (-r) host file(s)" << Endl;
		return 0;
	}

	net::Network::initialize();

	std::wstring host = cmdLine.getString(0);
	bool recursive = cmdLine.hasOption('r');

	Ref< net::TcpSocket > clientSocket = new net::TcpSocket();
	if (!clientSocket->connect(net::SocketAddressIPv4(host, c_serverPort)))
	{
		traktor::log::error << L"Unable to connect to \"" << host << L"\"" << Endl;
		return 1;
	}

	int32_t ret = 0;
	for (int i = 1; i < cmdLine.getCount(); ++i)
	{
		Path sourcePath = cmdLine.getString(i);
		if (!deployFiles(clientSocket, sourcePath, recursive))
		{
			ret = 2;
			break;
		}
	}

	clientSocket->close();
	clientSocket = 0;

	net::Network::finalize();
	return ret;
}
