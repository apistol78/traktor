#pragma once

#include "Ui/Form.h"

namespace traktor
{

class CommandLine;

	namespace ui
	{

class Menu;
class NotificationIcon;

	}

	namespace drone
	{

class DroneSettings;

class DroneForm : public ui::Form
{
	T_RTTI_CLASS

public:
	bool create(const CommandLine& cmdLine);

	void destroy();

private:
	Ref< ui::Menu > m_menuTools;
	Ref< ui::NotificationIcon > m_notificationIcon;
	Ref< DroneSettings > m_settings;
	bool m_toolExecuting;

	void eventNotificationButtonDown(ui::MouseButtonDownEvent* event);
};

	}
}

