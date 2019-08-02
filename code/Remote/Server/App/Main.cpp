#include <list>
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/System/OS.h"
#include "Core/Thread/Mutex.h"
#include "Net/Network.h"
#include "Remote/Server/App/Server.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Clipboard.h"
#include "Ui/Command.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/MessageBox.h"
#include "Ui/NotificationIcon.h"
#include "Ui/Events/AllEvents.h"

#if defined(_WIN32)
#	include "Ui/Win32/EventLoopWin32.h"
#	include "Ui/Win32/WidgetFactoryWin32.h"
#	undef MessageBox
#endif

#if defined(_WIN32)
#	include "Resources/NotificationBusy.h"
#	include "Resources/NotificationIdle.h"
#endif

using namespace traktor;

namespace
{

#if defined(_WIN32)
Mutex g_globalMutex(Guid(L"{DDB3D52F-8893-4f83-9FCD-D8A73211CC96}"));
Ref< ui::Menu > g_popupMenu;
Ref< ui::NotificationIcon > g_notificationIcon;

#endif

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
		new ui::WidgetFactoryWin32(),
		nullptr
	);
#endif

	if (cmdLine.getCount() <= 0)
	{
		log::error << L"Usage: Traktor.Remote.Server.App [-k|--keyword=(Filter keyword)] [-v|--verbose] (Scratch directory)" << Endl;
		return 1;
	}

	log::info << L"Traktor.Remote.Server.App; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;

	net::Network::initialize();

	Ref< remote::Server > server = new remote::Server();
	if (!server->create(
		cmdLine.getString(0),
		cmdLine.hasOption('k', L"keyword") ? cmdLine.getOption('k', L"keyword").getString() : L"",
		cmdLine.hasOption(L'v', L"verbose")
	))
	{
		log::error << L"Unable to create server object." << Endl;
		return 2;
	}

#if defined(_WIN32)
	g_popupMenu = new ui::Menu();
	g_popupMenu->add(new ui::MenuItem(ui::Command(L"RemoteServer.CopyScratch"), L"Copy Scratch Path"));
	g_popupMenu->add(new ui::MenuItem(ui::Command(L"RemoteServer.CleanScratch"), L"Clean Scratch"));
	g_popupMenu->add(new ui::MenuItem(ui::Command(L"RemoteServer.BrowseScratch"), L"Browse Scratch..."));
	g_popupMenu->add(new ui::MenuItem(L"-"));
	g_popupMenu->add(new ui::MenuItem(ui::Command(L"RemoteServer.Exit"), L"Exit"));

	Ref< ui::Bitmap > imageBusy = ui::Bitmap::load(c_ResourceNotificationBusy, sizeof(c_ResourceNotificationBusy), L"png");
	Ref< ui::Bitmap > imageIdle = ui::Bitmap::load(c_ResourceNotificationIdle, sizeof(c_ResourceNotificationIdle), L"png");

	g_notificationIcon = new ui::NotificationIcon();
	g_notificationIcon->create(L"Traktor.Remote.Server.App 2.1.0 (" + server->getScratchPath() + L")", imageIdle);
	g_notificationIcon->addEventHandler< ui::MouseButtonDownEvent >([&](ui::MouseButtonDownEvent* event) {
		const ui::MenuItem* item = g_popupMenu->showModal(nullptr, event->getPosition());
		if (!item)
			return;

		if (item->getCommand() == L"RemoteServer.CopyScratch")
		{
			ui::Clipboard* clipboard = ui::Application::getInstance()->getClipboard();
			if (clipboard)
				clipboard->setText(server->getScratchPath());
		}
		else if (item->getCommand() == L"RemoteServer.CleanScratch")
		{
			RefArray< File > files;
			FileSystem::getInstance().find(server->getScratchPath() + L"/*.*", files);
			for (RefArray< File >::const_iterator i = files.begin(); i != files.end(); ++i)
				FileSystem::getInstance().remove((*i)->getPath());
		}
		else if (item->getCommand() == L"RemoteServer.BrowseScratch")
		{
			OS::getInstance().exploreFile(server->getScratchPath());
		}
		else if (item->getCommand() == L"RemoteServer.Exit")
		{
			if (ui::MessageBox::show(L"Sure you want to exit RemoteServer?", L"Exit", ui::MbIconQuestion | ui::MbYesNo) == ui::DrYes)
				ui::Application::getInstance()->exit(0);
		}
	});
#endif

	log::info << L"Waiting for client(s)..." << Endl;

#if defined(_WIN32)
	while (ui::Application::getInstance()->process())
#else
	for (;;)
#endif
	{
#if defined(_WIN32)
		g_notificationIcon->setImage(imageBusy);
#endif
		if (!server->update())
			break;
#if defined(_WIN32)
		g_notificationIcon->setImage(imageIdle);
#endif
	}

#if defined(_WIN32)
	safeDestroy(g_notificationIcon);
#endif

	safeDestroy(server);

	net::Network::finalize();

#if defined(_WIN32)
	ui::Application::getInstance()->finalize();
#endif

	return 0;
}
