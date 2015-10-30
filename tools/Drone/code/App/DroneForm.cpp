#include <Ui/Application.h>
#include <Ui/NotificationIcon.h>
#include <Ui/Bitmap.h>
#include <Ui/Command.h>
#include <Ui/PopupMenu.h>
#include <Ui/MenuItem.h>
#include <Ui/MessageBox.h>
#include <Xml/XmlDeserializer.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/IStream.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Log/Log.h>
#include "App/DroneForm.h"
#include "App/DroneSettings.h"
#include "App/DroneTool.h"

// Resources
#include "Resources/TraktorTiny.h"

namespace traktor
{
	namespace drone
	{

const wchar_t c_title[] = { L"Traktor Drone v1.0" };

T_IMPLEMENT_RTTI_CLASS(L"traktor.drone.DroneForm", DroneForm, ui::Form)

bool DroneForm::create(const CommandLine& cmdLine)
{
	std::wstring configurationFile = L"Drone.App.config";

	if (cmdLine.getCount() >= 1)
		configurationFile = cmdLine.getString(0);

	if (!ui::Form::create(c_title, 0, 0, ui::WsNone))
		return false;

	Ref< IStream > file = FileSystem::getInstance().open(configurationFile, File::FmRead);
	if (!file)
	{
		log::error << L"Unable to open configuration \"" << configurationFile << L"\"" << Endl;
		return false;
	}

	m_settings = xml::XmlDeserializer(file).readObject< DroneSettings >();
	
	file->close();

	if (!m_settings)
	{
		log::error << L"Unable to read configuration \"" << cmdLine.getString(0) << L"\"" << Endl;
		return false;
	}

	m_menuTools = new ui::PopupMenu();
	m_menuTools->create();

	const RefArray< DroneTool >& tools = m_settings->getTools();
	for (RefArray< DroneTool >::const_iterator i = tools.begin(); i != tools.end(); ++i)
	{
		RefArray< ui::MenuItem > menuItems;
		(*i)->getMenuItems(menuItems);

		for (RefArray< ui::MenuItem >::iterator i = menuItems.begin(); i != menuItems.end(); ++i)
			m_menuTools->add(*i);
	}

	m_menuTools->add(new ui::MenuItem(L"-"));
	m_menuTools->add(new ui::MenuItem(ui::Command(L"Drone.Exit"), L"Exit"));

	m_notificationIcon = new ui::NotificationIcon();
	m_notificationIcon->create(c_title, ui::Bitmap::load(c_ResourceTraktorTiny, sizeof(c_ResourceTraktorTiny), L"png"));
	m_notificationIcon->addEventHandler< ui::MouseButtonDownEvent >(this, &DroneForm::eventNotificationButtonDown);

	m_toolExecuting = false;

	return true;
}

void DroneForm::destroy()
{
	m_menuTools->destroy();
	m_notificationIcon->destroy();
	ui::Form::destroy();
}

void DroneForm::eventNotificationButtonDown(ui::MouseButtonDownEvent* event)
{
	if (m_toolExecuting)
		return;

	setFocus();
	setForeground();

	Ref< ui::MenuItem > item = m_menuTools->show(this, screenToClient(event->getPosition()));

	if (!item)
		return;

	if (item->getCommand() == L"Drone.Exit")
	{
		if (ui::MessageBox::show(this, L"Sure you want to exit Drone?", L"Exit", ui::MbIconQuestion | ui::MbYesNo) == ui::DrYes)
			ui::Application::getInstance()->exit(0);
	}
	else
	{
		Ref< DroneTool > tool = item->getData< DroneTool >(L"TOOL");
		if (tool)
		{
			m_toolExecuting = true;

			if (!tool->execute(this, item))
				ui::MessageBox::show(this, L"Unable to execute tool", L"Error", ui::MbIconError | ui::MbOk);

			m_toolExecuting = false;
		}
	}
}

	}
}
