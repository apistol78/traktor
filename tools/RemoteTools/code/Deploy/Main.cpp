#include <Compress/Lzo/DeflateStreamLzo.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/Reader.h>
#include <Core/Io/StreamCopy.h>
#include <Core/Io/Writer.h>
#include <Core/Log/Log.h>
#include <Core/Misc/Adler32.h>
#include <Core/Misc/CommandLine.h>
#include <Core/System/OS.h>
#include <Core/Timer/Timer.h>
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

bool deployFile(net::TcpSocket* clientSocket, const Path& sourceFile, const Path& targetBase)
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

	if (!targetBase.getPathName().empty())
		targetFile = targetBase + targetFile;

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
			traktor::log::error << L"Unable to deploy file \"" << sourceFile.getPathName() << L"\"; unable to transmit" << Endl;
			return false;
		}
	}
	else
		traktor::log::info << L"File already up-to-date; skipped" << Endl;

	fileStream->close();
	fileStream = 0;

	traktor::log::info << L"File deployed successfully" << Endl;
	return true;
}

bool deployFiles(net::TcpSocket* clientSocket, const Path& sourcePath, const Path& targetBase, bool recursive)
{
	RefArray< File > files;

	FileSystem::getInstance().find(sourcePath, files);
	traktor::log::info << L"Found " << int32_t(files.size()) << L" file(s) matching \"" << sourcePath.getPathName() << L"\"" << Endl;

	for (RefArray< File >::iterator i = files.begin(); i != files.end(); ++i)
	{
		Path sourceFile = (*i)->getPath();
		if ((*i)->isDirectory())
		{
			if (recursive)
			{
				if (sourceFile.getFileName() != L"." && sourceFile.getFileName() != L"..")
				{
					traktor::log::info << L"Enter directory \"" << sourceFile.getPathName() << L"\"" << Endl;
					if (!deployFiles(clientSocket, sourceFile.getPathName() + L"/" + sourcePath.getFileName(), targetBase, true))
						return false;
					traktor::log::info << L"Leaving directory \"" << sourceFile.getPathName() << L"\"" << Endl;
				}
			}
			else
				traktor::log::info << L"Directory \"" << sourceFile.getPathName() << L"\" skipped" << Endl;
		}
		else
		{
			if (!deployFile(clientSocket, sourceFile, targetBase))
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
		traktor::log::info << L"Usage: RemoteDeploy (-r, -recursive) host file(s)" << Endl;
		traktor::log::info << L"   -r, -recursive   Recursive deploy files in directories." << Endl;
		traktor::log::info << L"   -t, -target-base Target base path." << Endl;
		return 0;
	}

	log::info << L"RemoteDeploy; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;

	net::Network::initialize();

	std::wstring host = cmdLine.getString(0);
	bool recursive = cmdLine.hasOption('r', L"recursive");
	bool base = cmdLine.hasOption('t', L"target-base");
	Path targetBase = base ? cmdLine.getOption('t', L"target-base").getString() : L"";

	if (!targetBase.isRelative())
	{
		traktor::log::info << L"Target base must be a relative path" << Endl;
		return 1;
	}

	log::info << L"Host \"" << host << L"\"" << Endl;
	log::info << L"Recursive " << (recursive ? L"YES" : L"NO") << Endl;
	log::info << int32_t(cmdLine.getCount()) << L" command line value(s)" << Endl;
	log::info << L"Target base \"" << targetBase.getPathName() << L"\"" << Endl;

	Ref< net::TcpSocket > clientSocket = new net::TcpSocket();
	if (!clientSocket->connect(net::SocketAddressIPv4(host, c_serverPort)))
	{
		traktor::log::info << L"Unable to connect to \"" << host << L"\"" << Endl;
		return 2;
	}

	traktor::log::info << L"Successfully connected to server" << Endl;

	int32_t ret = 0;
	for (int i = 1; i < cmdLine.getCount(); ++i)
	{
		Path sourcePath = cmdLine.getString(i);
		if (!deployFiles(clientSocket, sourcePath, targetBase, recursive))
		{
			ret = 3;
			break;
		}
	}

	clientSocket->close();
	clientSocket = 0;

	net::Network::finalize();
	return ret;
}
