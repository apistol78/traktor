#include <list>
#include <Compress/Lzo/InflateStreamLzo.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/Reader.h>
#include <Core/Io/StreamCopy.h>
#include <Core/Io/Writer.h>
#include <Core/Log/Log.h>
#include <Core/Misc/Adler32.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Misc/SafeDestroy.h>
#include <Core/Settings/PropertyInteger.h>
#include <Core/Settings/PropertyString.h>
#include <Core/Settings/PropertyStringArray.h>
#include <Core/System/IProcess.h>
#include <Core/System/OS.h>
#include <Core/Thread/Mutex.h>
#include <Core/Thread/Thread.h>
#include <Core/Thread/ThreadManager.h>
#include <Net/Network.h>
#include <Net/SocketAddressIPv4.h>
#include <Net/SocketStream.h>
#include <Net/TcpSocket.h>
#include <Net/Discovery/DiscoveryManager.h>
#include <Net/Discovery/NetworkService.h>
#include <Ui/Application.h>
#include <Ui/Bitmap.h>
#include <Ui/Clipboard.h>
#include <Ui/Command.h>
#include <Ui/MenuItem.h>
#include <Ui/MessageBox.h>
#include <Ui/NotificationIcon.h>
#include <Ui/PopupMenu.h>
#include <Ui/Events/AllEvents.h>
#if defined(_WIN32)
#	include <Ui/Win32/EventLoopWin32.h>
#	include <Ui/Win32/WidgetFactoryWin32.h>
#	undef MessageBox
#endif

#if defined(_WIN32)
#	include "Resources/NotificationBusy.h"
#	include "Resources/NotificationIdle.h"
#endif

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

std::wstring g_scratchPath;
std::map< std::wstring, uint32_t > g_fileHashes;

#if defined(_WIN32)
Mutex g_globalMutex(Guid(L"{DDB3D52F-8893-4f83-9FCD-D8A73211CC96}"));
Ref< ui::PopupMenu > g_popupMenu;
Ref< ui::NotificationIcon > g_notificationIcon;

void eventNotificationButtonDown(ui::MouseButtonDownEvent* event)
{
	Ref< ui::MenuItem > item = g_popupMenu->show(event->getPosition());
	if (!item)
		return;

	if (item->getCommand() == L"RemoteServer.CopyScratch")
	{
		ui::Clipboard* clipboard = ui::Application::getInstance()->getClipboard();
		if (clipboard)
			clipboard->setText(g_scratchPath);
	}
	else if (item->getCommand() == L"RemoteServer.CleanScratch")
	{
		RefArray< File > files;
		FileSystem::getInstance().find(g_scratchPath + L"/*.*", files);
		for (RefArray< File >::const_iterator i = files.begin(); i != files.end(); ++i)
			FileSystem::getInstance().remove((*i)->getPath());
	}
	else if (item->getCommand() == L"RemoteServer.BrowseScratch")
	{
		OS::getInstance().exploreFile(g_scratchPath);
	}
	else if (item->getCommand() == L"RemoteServer.Exit")
	{
		if (ui::MessageBox::show(L"Sure you want to exit RemoteServer?", L"Exit", ui::MbIconQuestion | ui::MbYesNo) == ui::DrYes)
			ui::Application::getInstance()->exit(0);
	}
}
#endif

uint8_t handleDeploy(net::TcpSocket* clientSocket)
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

	traktor::log::info << L"Receiving file \"" << pathName << L"\", " << size << L" byte(s)" << Endl;

	Path path(g_scratchPath + L"/" + user + L"/" + pathName);
	bool outOfSync = true;

	std::map< std::wstring, uint32_t >::const_iterator i = g_fileHashes.find(path.getPathName());
	if (i != g_fileHashes.end())
	{
		// File has already been hashed once; check only against previous hash.
		if (i->second == hash)
		{
			// Hashes match; finally ensure file still exist, could have been manually removed.
			if (FileSystem::getInstance().exist(path))
			{
				traktor::log::info << L"File up-to-date; skipping (1)" << Endl;
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
			int32_t nread;
			while ((nread = fileStream->read(buffer, sizeof(buffer))) > 0)
				adler.feed(buffer, nread);

			adler.end();

			fileStream->close();
			fileStream = 0;

			if (adler.get() == hash)
			{
				traktor::log::info << L"File up-to-date; skipping (2)" << Endl;
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
			traktor::log::error << L"Unable to create file \"" << pathName << L"\"" << Endl;
			return c_errIoFailed;
		}

		compress::InflateStreamLzo inflateStream(&clientStream);
		if (!StreamCopy(fileStream, &inflateStream).execute(size))
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

	g_fileHashes[path.getPathName()] = hash;

	return c_errNone;
}

uint8_t handleLaunchProcess(net::Socket* clientSocket)
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

	traktor::log::info << L"Launching \"" << pathName << L"\"" << Endl;
	traktor::log::info << L"\targuments \"" << arguments << L"\"" << Endl;

	Path path(g_scratchPath + L"/" + user + L"/" + pathName);
	Ref< IProcess > process = OS::getInstance().execute(L"\"" + path.getPathName() + L"\" " + arguments, g_scratchPath + L"/" + user, 0, false, false, false);
	if (!process)
	{
		traktor::log::error << L"Unable to launch process \"" << pathName << L"\"" << Endl;
		writer << uint8_t(c_errLaunchFailed);
		return c_errLaunchFailed;
	}

	if (wait)
	{
		process->wait();
		exitCode = process->exitCode();
	}

	writer << uint8_t(c_errNone);
	return c_errNone;
}

void processClient(Ref< net::TcpSocket > clientSocket)
{
	uint8_t msg;
	uint8_t ret;

	if (clientSocket->select(true, false, false, 5000) <= 0)
	{
		traktor::log::info << L"Client terminated unexpectedly (1)." << Endl;
		return;
	}

	int32_t md = clientSocket->recv();
	if (md < 0)
	{
		traktor::log::info << L"Client terminated unexpectedly (2)." << Endl;
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

	default:
		traktor::log::error << L"Invalid message ID from client; " << int32_t(msg) << Endl;
		ret = c_errUnknown;
		break;
	}

	clientSocket->close();
	clientSocket = 0;
}

}

#if !defined(_WIN32) || defined(_CONSOLE)
int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR szCmdLine, int)
{
	std::vector< std::wstring > argv;

	// If global mutex already was created then
	// the server is already running.
	if (g_globalMutex.existing())
		return 0;

	TCHAR szFilename[MAX_PATH] = _T("");
	GetModuleFileName(NULL, szFilename, sizeof(szFilename));
	argv.push_back(tstows(szFilename));

	Split< std::wstring >::any(mbstows(szCmdLine), L" \t", argv);
	CommandLine cmdLine(argv);
#endif

#if defined(_WIN32)
	ui::Application::getInstance()->initialize(
		new ui::EventLoopWin32(),
		new ui::WidgetFactoryWin32()
	);
#endif

	T_FORCE_LINK_REF(PropertyInteger);

	traktor::log::info << L"Traktor RemoteServer 1.91" << Endl;

	if (cmdLine.getCount() <= 0)
	{
		traktor::log::error << L"Usage: RemoteServer [-k|--keyword=(Filter keyword)] (Scratch directory)" << Endl;
		return 1;
	}

	g_scratchPath = cmdLine.getString(0);

#if defined(_WIN32)
	g_popupMenu = new ui::PopupMenu();
	g_popupMenu->create();
	g_popupMenu->add(new ui::MenuItem(ui::Command(L"RemoteServer.CopyScratch"), L"Copy Scratch Path"));
	g_popupMenu->add(new ui::MenuItem(ui::Command(L"RemoteServer.CleanScratch"), L"Clean Scratch"));
	g_popupMenu->add(new ui::MenuItem(ui::Command(L"RemoteServer.BrowseScratch"), L"Browse Scratch..."));
	g_popupMenu->add(new ui::MenuItem(L"-"));
	g_popupMenu->add(new ui::MenuItem(ui::Command(L"RemoteServer.Exit"), L"Exit"));

	Ref< ui::Bitmap > imageBusy = ui::Bitmap::load(c_ResourceNotificationBusy, sizeof(c_ResourceNotificationBusy), L"png");
	Ref< ui::Bitmap > imageIdle = ui::Bitmap::load(c_ResourceNotificationIdle, sizeof(c_ResourceNotificationIdle), L"png");

	g_notificationIcon = new ui::NotificationIcon();
	g_notificationIcon->create(L"Traktor RemoteServer 1.91 (" + g_scratchPath + L")", imageIdle);
	g_notificationIcon->addEventHandler< ui::MouseButtonDownEvent >(&eventNotificationButtonDown);
#endif

	net::Network::initialize();

	// Create server socket.
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

	// Create discovery manager and publish ourself.
	std::wstring hostName = L"";
	std::vector< std::wstring > platforms;

#if defined(_WIN32)
	platforms.push_back(L"Mobile6");
	platforms.push_back(L"PS3");
	platforms.push_back(L"Win32");
	platforms.push_back(L"Win64");
	platforms.push_back(L"XBox360");
	platforms.push_back(L"Emscripten");
	platforms.push_back(L"Android");
	platforms.push_back(L"PNaCl");
#elif defined(__APPLE__)
	platforms.push_back(L"iPad");
	platforms.push_back(L"iPhone");
	platforms.push_back(L"Mac OSX");
	platforms.push_back(L"Emscripten");
	platforms.push_back(L"Android");
	platforms.push_back(L"PNaCl");
#elif defined(__LINUX__)
	platforms.push_back(L"Linux32");
	platforms.push_back(L"Linux64");
	platforms.push_back(L"Emscripten");
	platforms.push_back(L"Android");
	platforms.push_back(L"PNaCl");
#endif

	Ref< net::DiscoveryManager > discoveryManager = new net::DiscoveryManager();
	if (!discoveryManager->create(net::MdPublishServices))
	{
		traktor::log::error << L"Unable to create discovery manager" << Endl;
		return 4;
	}

	traktor::log::info << L"Waiting for client(s)..." << Endl;

#if defined(_WIN32)
	while (ui::Application::getInstance()->process())
#else
	for (;;)
#endif
	{
		// Keep published interface up-to-date.
		net::SocketAddressIPv4::Interface itf;
		if (!net::SocketAddressIPv4::getBestInterface(itf))
		{
			traktor::log::error << L"Unable to get interfaces" << Endl;
			return 4;
		}

		if (itf.addr->getHostName() != hostName)
		{
			traktor::log::info << L"Discoverable as \"RemoteTools/Server\", host \"" << itf.addr->getHostName() << L"\"" << Endl;

			Ref< PropertyGroup > properties = new PropertyGroup();
			properties->setProperty< PropertyString >(L"Host", itf.addr->getHostName());
			properties->setProperty< PropertyString >(L"Description", OS::getInstance().getComputerName());
			properties->setProperty< PropertyStringArray >(L"Platforms", platforms);

			if (cmdLine.hasOption('k', L"keyword"))
				properties->setProperty< PropertyString >(L"Keyword", cmdLine.getOption('k', L"keyword").getString());

			discoveryManager->removeAllServices();
			discoveryManager->addService(new net::NetworkService(L"RemoteTools/Server", properties));

			hostName = itf.addr->getHostName();
		}

		if (serverSocket->select(true, false, false, 1000) <= 0)
			continue;

		Ref< net::TcpSocket > clientSocket = serverSocket->accept();
		if (!clientSocket)
			continue;

#if defined(_WIN32)
		g_notificationIcon->setImage(imageBusy);
#endif

        processClient(clientSocket);

#if defined(_WIN32)
		g_notificationIcon->setImage(imageIdle);
#endif
	}

#if defined(_WIN32)
	safeDestroy(g_notificationIcon);
	safeDestroy(g_popupMenu);
#endif

	net::Network::finalize();

#if defined(_WIN32)
	ui::Application::getInstance()->finalize();
#endif

	return 0;
}
