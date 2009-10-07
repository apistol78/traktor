#include "Ui/Cocoa/WidgetFactoryCocoa.h"
#include "Ui/Cocoa/ButtonCocoa.h"
#include "Ui/Cocoa/ContainerCocoa.h"
#include "Ui/Cocoa/FormCocoa.h"
#include "Ui/Cocoa/ListBoxCocoa.h"
#include "Ui/Cocoa/UserWidgetCocoa.h"

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
	return 0;
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
	return 0;
}

IMessageBox* WidgetFactoryCocoa::createMessageBox(EventSubject* owner)
{
	return 0;
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
	return 0;
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
	return 0;
}

ISlider* WidgetFactoryCocoa::createSlider(EventSubject* owner)
{
	return 0;
}

IStatic* WidgetFactoryCocoa::createStatic(EventSubject* owner)
{
	return 0;
}

IToolForm* WidgetFactoryCocoa::createToolForm(EventSubject* owner)
{
	return 0;
}

ITreeView* WidgetFactoryCocoa::createTreeView(EventSubject* owner)
{
	return 0;
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
	return 0;
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
