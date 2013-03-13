#include <Core/Io/FileSystem.h>
#include <Core/Io/Reader.h>
#include <Core/Io/StreamCopy.h>
#include <Core/Io/Writer.h>
#include <Core/Log/Log.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Misc/String.h>
#include <Core/System/OS.h>
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

}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	if (argc < 3)
	{
		log::info << L"Usage: RemoteLaunch (-w) host application arguments" << Endl;
		return 0;
	}

	log::info << L"RemoteLaunch; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;

	net::Network::initialize();

	std::wstring host = cmdLine.getString(0);
	std::wstring application = cmdLine.getString(1);
	bool wait = cmdLine.hasOption('w');

	std::wstring arguments;
	for (int32_t i = 2; i < cmdLine.getCount(); ++i)
	{
		if (i > 2)
			arguments += L" ";

		std::wstring argument = cmdLine.getString(i);
		if (startsWith< std::wstring >(argument, L"\\-"))
			argument = argument.substr(1);

		arguments += argument;
	}

	Ref< net::TcpSocket > clientSocket = new net::TcpSocket();
	if (!clientSocket->connect(net::SocketAddressIPv4(host, c_serverPort)))
	{
		traktor::log::error << L"Unable to connect to \"" << host << L"\"" << Endl;
		return 1;
	}

	traktor::log::info << L"Successfully connected to server" << Endl;

	net::SocketStream clientStream(clientSocket);
	Writer writer(&clientStream);
	Reader reader(&clientStream);
	uint8_t ret;

	writer << c_msgLaunchProcess;
	writer << OS::getInstance().getCurrentUser();
	writer << application;
	writer << arguments;
	writer << wait;

	reader >> ret;
	if (ret != c_errNone)
		traktor::log::error << L"Unable to launch \"" << application << L"\"; server error " << int32_t(ret) << Endl;

	clientSocket->close();
	clientSocket = 0;

	net::Network::finalize();
	return 0;
}
