#include "Ui/Gtk/BitmapGtk.h"
#include "Ui/Gtk/ContainerGtk.h"
#include "Ui/Gtk/ClipboardGtk.h"
#include "Ui/Gtk/DialogGtk.h"
#include "Ui/Gtk/FileDialogGtk.h"
#include "Ui/Gtk/FormGtk.h"
#include "Ui/Gtk/ToolFormGtk.h"
#include "Ui/Gtk/UserWidgetGtk.h"
#include "Ui/Gtk/WidgetFactoryGtk.h"

namespace traktor
{
	namespace ui
	{

IContainer* WidgetFactoryGtk::createContainer(EventSubject* owner)
{
	return new ContainerGtk(owner);
}

IDialog* WidgetFactoryGtk::createDialog(EventSubject* owner)
{
	return new DialogGtk(owner);
}

IFileDialog* WidgetFactoryGtk::createFileDialog(EventSubject* owner)
{
	return new FileDialogGtk(owner);
}

IForm* WidgetFactoryGtk::createForm(EventSubject* owner)
{
	return new FormGtk(owner);
}

IMessageBox* WidgetFactoryGtk::createMessageBox(EventSubject* owner)
{
	return 0;
}

INotificationIcon* WidgetFactoryGtk::createNotificationIcon(EventSubject* owner)
{
	return 0;
}

IPathDialog* WidgetFactoryGtk::createPathDialog(EventSubject* owner)
{
	return 0;
}

IToolForm* WidgetFactoryGtk::createToolForm(EventSubject* owner)
{
	return new ToolFormGtk(owner);
}

IUserWidget* WidgetFactoryGtk::createUserWidget(EventSubject* owner)
{
	return new UserWidgetGtk(owner);
}

IWebBrowser* WidgetFactoryGtk::createWebBrowser(EventSubject* owner)
{
	return 0;
}

ISystemBitmap* WidgetFactoryGtk::createBitmap()
{
	return new BitmapGtk();
}

IClipboard* WidgetFactoryGtk::createClipboard()
{
	return new ClipboardGtk();
}

int32_t WidgetFactoryGtk::getSystemDPI() const
{
	return 96;
}

bool WidgetFactoryGtk::getSystemColor(SystemColor systemColor, Color4ub& outColor)
{
	return false;
}

void WidgetFactoryGtk::getSystemFonts(std::list< std::wstring >& outFonts)
{
}

	}
}

