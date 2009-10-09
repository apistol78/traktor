#include "Ui/Cocoa/WidgetFactoryCocoa.h"
#include "Ui/Cocoa/ButtonCocoa.h"
#include "Ui/Cocoa/CheckBoxCocoa.h"
#include "Ui/Cocoa/ContainerCocoa.h"
#include "Ui/Cocoa/FormCocoa.h"
#include "Ui/Cocoa/ListBoxCocoa.h"
#include "Ui/Cocoa/MenuBarCocoa.h"
#include "Ui/Cocoa/MessageBoxCocoa.h"
#include "Ui/Cocoa/PopupMenuCocoa.h"
#include "Ui/Cocoa/ScrollBarCocoa.h"
#include "Ui/Cocoa/StaticCocoa.h"
#include "Ui/Cocoa/ToolFormCocoa.h"
#include "Ui/Cocoa/TreeViewCocoa.h"
#include "Ui/Cocoa/UserWidgetCocoa.h"
#include "Ui/Cocoa/BitmapCocoa.h"

namespace traktor
{
	namespace ui
	{

IButton* WidgetFactoryCocoa::createButton(EventSubject* owner)
{
	return new ButtonCocoa(owner);
}

ICheckBox* WidgetFactoryCocoa::createCheckBox(EventSubject* owner)
{
	return new CheckBoxCocoa(owner);
}

IComboBox* WidgetFactoryCocoa::createComboBox(EventSubject* owner)
{
	return 0;
}

IContainer* WidgetFactoryCocoa::createContainer(EventSubject* owner)
{
	return new ContainerCocoa(owner);
}

IDialog* WidgetFactoryCocoa::createDialog(EventSubject* owner)
{
	return 0;
}

IDropDown* WidgetFactoryCocoa::createDropDown(EventSubject* owner)
{
	return 0;
}

IEdit* WidgetFactoryCocoa::createEdit(EventSubject* owner)
{
	return 0;
}

IFileDialog* WidgetFactoryCocoa::createFileDialog(EventSubject* owner)
{
	return 0;
}

IForm* WidgetFactoryCocoa::createForm(EventSubject* owner)
{
	return new FormCocoa(owner);
}

IListBox* WidgetFactoryCocoa::createListBox(EventSubject* owner)
{
	return new ListBoxCocoa(owner);
}

IListView* WidgetFactoryCocoa::createListView(EventSubject* owner)
{
	return 0;
}

IMenuBar* WidgetFactoryCocoa::createMenuBar(EventSubject* owner)
{
	return new MenuBarCocoa(owner);
}

IMessageBox* WidgetFactoryCocoa::createMessageBox(EventSubject* owner)
{
	return new MessageBoxCocoa(owner);
}

INotificationIcon* WidgetFactoryCocoa::createNotificationIcon(EventSubject* owner)
{
	return 0;
}

IPanel* WidgetFactoryCocoa::createPanel(EventSubject* owner)
{
	return 0;
}

IPopupMenu* WidgetFactoryCocoa::createPopupMenu(EventSubject* owner)
{
	return new PopupMenuCocoa(owner);
}

IRadioButton* WidgetFactoryCocoa::createRadioButton(EventSubject* owner)
{
	return 0;
}

IRichEdit* WidgetFactoryCocoa::createRichEdit(EventSubject* owner)
{
	return 0;
}

IScrollBar* WidgetFactoryCocoa::createScrollBar(EventSubject* owner)
{
	return new ScrollBarCocoa(owner);
}

ISlider* WidgetFactoryCocoa::createSlider(EventSubject* owner)
{
	return 0;
}

IStatic* WidgetFactoryCocoa::createStatic(EventSubject* owner)
{
	return new StaticCocoa(owner);
}

IToolForm* WidgetFactoryCocoa::createToolForm(EventSubject* owner)
{
	return new ToolFormCocoa(owner);
}

ITreeView* WidgetFactoryCocoa::createTreeView(EventSubject* owner)
{
	return new TreeViewCocoa(owner);
}

IUserWidget* WidgetFactoryCocoa::createUserWidget(EventSubject* owner)
{
	return new UserWidgetCocoa(owner);
}

INative* WidgetFactoryCocoa::createNative(EventSubject* owner)
{
	return 0;
}

IBitmap* WidgetFactoryCocoa::createBitmap()
{
	return new BitmapCocoa();
}

IClipboard* WidgetFactoryCocoa::createClipboard()
{
	return 0;
}

bool WidgetFactoryCocoa::getSystemColor(SystemColor systemColor, Color& outColor)
{
	return false;
}

	}
}
