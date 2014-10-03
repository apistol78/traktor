#ifndef traktor_drone_DroneForm_H
#define traktor_drone_DroneForm_H

#include <Ui/Form.h>
#include <Ui/NotificationIcon.h>

namespace traktor
{

class CommandLine;

	namespace ui
	{

class PopupMenu;
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
	Ref< ui::PopupMenu > m_menuTools;
	Ref< ui::NotificationIcon > m_notificationIcon;
	Ref< DroneSettings > m_settings;
	bool m_toolExecuting;

	void eventNotificationButtonDown(ui::MouseButtonDownEvent* event);
};

	}
}

#endif	// traktor_drone_DroneForm_H
