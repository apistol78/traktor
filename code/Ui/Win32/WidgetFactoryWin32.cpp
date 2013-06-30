#include "Ui/Win32/WidgetFactoryWin32.h"
#include "Ui/Win32/ButtonWin32.h"
#include "Ui/Win32/CheckBoxWin32.h"
#include "Ui/Win32/ComboBoxWin32.h"
#include "Ui/Win32/ContainerWin32.h"
#include "Ui/Win32/DialogWin32.h"
#include "Ui/Win32/DropDownWin32.h"
#include "Ui/Win32/EditWin32.h"
#include "Ui/Win32/FileDialogWin32.h"
#include "Ui/Win32/FormWin32.h"
#include "Ui/Win32/ListBoxWin32.h"
#include "Ui/Win32/ListViewWin32.h"
#include "Ui/Win32/MenuBarWin32.h"
#include "Ui/Win32/MessageBoxWin32.h"
#include "Ui/Win32/NotificationIconWin32.h"
#include "Ui/Win32/PanelWin32.h"
#include "Ui/Win32/PopupMenuWin32.h"
#include "Ui/Win32/RadioButtonWin32.h"
#include "Ui/Win32/RichEditWin32.h"
#include "Ui/Win32/ScrollBarWin32.h"
#include "Ui/Win32/SliderWin32.h"
#include "Ui/Win32/StaticWin32.h"
#include "Ui/Win32/ToolFormWin32.h"
#include "Ui/Win32/TreeViewWin32.h"
#include "Ui/Win32/UserWidgetWin32.h"
#include "Ui/Win32/WebBrowserWin32.h"
#include "Ui/Win32/NativeWin32.h"
#include "Ui/Win32/BitmapWin32.h"
#include "Ui/Win32/ClipboardWin32.h"

namespace traktor
{
	namespace ui
	{

IButton* WidgetFactoryWin32::createButton(EventSubject* owner)
{
	return new ButtonWin32(owner);
}

ICheckBox* WidgetFactoryWin32::createCheckBox(EventSubject* owner)
{
	return new CheckBoxWin32(owner);
}

IComboBox* WidgetFactoryWin32::createComboBox(EventSubject* owner)
{
	return new ComboBoxWin32(owner);
}

IContainer* WidgetFactoryWin32::createContainer(EventSubject* owner)
{
	return new ContainerWin32(owner);
}

IDialog* WidgetFactoryWin32::createDialog(EventSubject* owner)
{
	return new DialogWin32(owner);
}

IDropDown* WidgetFactoryWin32::createDropDown(EventSubject* owner)
{
	return new DropDownWin32(owner);
}

IEdit* WidgetFactoryWin32::createEdit(EventSubject* owner)
{
	return new EditWin32(owner);
}

IFileDialog* WidgetFactoryWin32::createFileDialog(EventSubject* owner)
{
	return new FileDialogWin32(owner);
}

IForm* WidgetFactoryWin32::createForm(EventSubject* owner)
{
	return new FormWin32(owner);
}

IListBox* WidgetFactoryWin32::createListBox(EventSubject* owner)
{
	return new ListBoxWin32(owner);
}

IListView* WidgetFactoryWin32::createListView(EventSubject* owner)
{
	return new ListViewWin32(owner);
}

IMenuBar* WidgetFactoryWin32::createMenuBar(EventSubject* owner)
{
	return new MenuBarWin32(owner);
}

IMessageBox* WidgetFactoryWin32::createMessageBox(EventSubject* owner)
{
	return new MessageBoxWin32(owner);
}

INotificationIcon* WidgetFactoryWin32::createNotificationIcon(EventSubject* owner)
{
	return new NotificationIconWin32(owner);
}

IPanel* WidgetFactoryWin32::createPanel(EventSubject* owner)
{
	return new PanelWin32(owner);
}

IPopupMenu* WidgetFactoryWin32::createPopupMenu(EventSubject* owner)
{
	return new PopupMenuWin32(/*owner*/);
}

IRadioButton* WidgetFactoryWin32::createRadioButton(EventSubject* owner)
{
	return new RadioButtonWin32(owner);
}

IRichEdit* WidgetFactoryWin32::createRichEdit(EventSubject* owner)
{
	return new RichEditWin32(owner);
}

IScrollBar* WidgetFactoryWin32::createScrollBar(EventSubject* owner)
{
	return new ScrollBarWin32(owner);
}

ISlider* WidgetFactoryWin32::createSlider(EventSubject* owner)
{
	return new SliderWin32(owner);
}

IStatic* WidgetFactoryWin32::createStatic(EventSubject* owner)
{
	return new StaticWin32(owner);
}

IToolForm* WidgetFactoryWin32::createToolForm(EventSubject* owner)
{
	return new ToolFormWin32(owner);
}

ITreeView* WidgetFactoryWin32::createTreeView(EventSubject* owner)
{
	return new TreeViewWin32(owner);
}

IUserWidget* WidgetFactoryWin32::createUserWidget(EventSubject* owner)
{
	return new UserWidgetWin32(owner);
}

IWebBrowser* WidgetFactoryWin32::createWebBrowser(EventSubject* owner)
{
	return new WebBrowserWin32(owner);
}

INative* WidgetFactoryWin32::createNative(EventSubject* owner)
{
	return new NativeWin32(owner);
}

IBitmap* WidgetFactoryWin32::createBitmap()
{
	return new BitmapWin32();
}

IClipboard* WidgetFactoryWin32::createClipboard()
{
	return new ClipboardWin32();
}

bool WidgetFactoryWin32::getSystemColor(SystemColor systemColor, Color4ub& outColor)
{
	const int c_systemColors[] =
	{
		COLOR_BACKGROUND,
		COLOR_ACTIVECAPTION,
		COLOR_INACTIVECAPTION,
		COLOR_MENU,
		COLOR_WINDOW,
		COLOR_WINDOWFRAME,
		COLOR_MENUTEXT,
		COLOR_WINDOWTEXT,
		COLOR_CAPTIONTEXT,
		COLOR_INACTIVECAPTIONTEXT,
		COLOR_ACTIVEBORDER,
		COLOR_INACTIVEBORDER,
		COLOR_BTNFACE,
		COLOR_BTNSHADOW,
		COLOR_GRAYTEXT
	};

	DWORD c = GetSysColor(c_systemColors[systemColor]);
	outColor = Color4ub(
		GetRValue(c),
		GetGValue(c),
		GetBValue(c)
	);

	return true;
}

	}
}
