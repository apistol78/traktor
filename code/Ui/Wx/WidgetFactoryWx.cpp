#include "Core/Log/Log.h"
#include "Ui/Wx/WidgetFactoryWx.h"
#include "Ui/Wx/ButtonWx.h"
#include "Ui/Wx/CheckBoxWx.h"
#include "Ui/Wx/ComboBoxWx.h"
#include "Ui/Wx/ContainerWx.h"
#include "Ui/Wx/DialogWx.h"
#include "Ui/Wx/DropDownWx.h"
#include "Ui/Wx/EditWx.h"
#include "Ui/Wx/FileDialogWx.h"
#include "Ui/Wx/FormWx.h"
#include "Ui/Wx/ListBoxWx.h"
#include "Ui/Wx/ListViewWx.h"
#include "Ui/Wx/MenuBarWx.h"
#include "Ui/Wx/MessageBoxWx.h"
#include "Ui/Wx/PanelWx.h"
#include "Ui/Wx/PopupMenuWx.h"
#include "Ui/Wx/RadioButtonWx.h"
#include "Ui/Wx/RichEditWx.h"
#include "Ui/Wx/ScrollBarWx.h"
#include "Ui/Wx/SliderWx.h"
#include "Ui/Wx/StaticWx.h"
#include "Ui/Wx/ToolFormWx.h"
#include "Ui/Wx/TreeViewWx.h"
#include "Ui/Wx/UserWidgetWx.h"
#include "Ui/Wx/BitmapWx.h"

namespace traktor
{
	namespace ui
	{

IButton* WidgetFactoryWx::createButton(EventSubject* owner)
{
	return new ButtonWx(owner);
}

ICheckBox* WidgetFactoryWx::createCheckBox(EventSubject* owner)
{
	return new CheckBoxWx(owner);
}

IComboBox* WidgetFactoryWx::createComboBox(EventSubject* owner)
{
	return new ComboBoxWx(owner);
}

IContainer* WidgetFactoryWx::createContainer(EventSubject* owner)
{
	return new ContainerWx(owner);
}

IDialog* WidgetFactoryWx::createDialog(EventSubject* owner)
{
	return new DialogWx(owner);
}

IDropDown* WidgetFactoryWx::createDropDown(EventSubject* owner)
{
	return new DropDownWx(owner);
}

IEdit* WidgetFactoryWx::createEdit(EventSubject* owner)
{
	return new EditWx(owner);
}

IFileDialog* WidgetFactoryWx::createFileDialog(EventSubject* owner)
{
	return new FileDialogWx(owner);
}

IForm* WidgetFactoryWx::createForm(EventSubject* owner)
{
	return new FormWx(owner);
}

IListBox* WidgetFactoryWx::createListBox(EventSubject* owner)
{
	return new ListBoxWx(owner);
}

IListView* WidgetFactoryWx::createListView(EventSubject* owner)
{
	return new ListViewWx(owner);
}

IMenuBar* WidgetFactoryWx::createMenuBar(EventSubject* owner)
{
	return new MenuBarWx(owner);
}

IMessageBox* WidgetFactoryWx::createMessageBox(EventSubject* owner)
{
	return new MessageBoxWx(owner);
}

INotificationIcon* WidgetFactoryWx::createNotificationIcon(EventSubject* owner)
{
	return 0;
}

IPanel* WidgetFactoryWx::createPanel(EventSubject* owner)
{
	return new PanelWx(owner);
}

IPathDialog* WidgetFactoryWx::createPathDialog(EventSubject* owner)
{
	return 0;
}

IPopupMenu* WidgetFactoryWx::createPopupMenu(EventSubject* owner)
{
	return new PopupMenuWx(owner);
}

IRadioButton* WidgetFactoryWx::createRadioButton(EventSubject* owner)
{
	return new RadioButtonWx(owner);
}

IRichEdit* WidgetFactoryWx::createRichEdit(EventSubject* owner)
{
	return new RichEditWx(owner);
}

IScrollBar* WidgetFactoryWx::createScrollBar(EventSubject* owner)
{
	return new ScrollBarWx(owner);
}

ISlider* WidgetFactoryWx::createSlider(EventSubject* owner)
{
	return new SliderWx(owner);
}

IStatic* WidgetFactoryWx::createStatic(EventSubject* owner)
{
	return new StaticWx(owner);
}

IToolForm* WidgetFactoryWx::createToolForm(EventSubject* owner)
{
	return new ToolFormWx(owner);
}

ITreeView* WidgetFactoryWx::createTreeView(EventSubject* owner)
{
	return new TreeViewWx(owner);
}

IUserWidget* WidgetFactoryWx::createUserWidget(EventSubject* owner)
{
	return new UserWidgetWx(owner);
}

IWebBrowser* WidgetFactoryWx::createWebBrowser(EventSubject* owner)
{
	return 0;
}

INative* WidgetFactoryWx::createNative(EventSubject* owner)
{
	return 0;
}

ISystemBitmap* WidgetFactoryWx::createBitmap()
{
	return new BitmapWx();
}

IClipboard* WidgetFactoryWx::createClipboard()
{
	return 0;
}

int32_t WidgetFactoryWx::getSystemDPI() const
{
	wxScreenDC dc;
	return dc.GetPPI().GetHeight();
}

bool WidgetFactoryWx::getSystemColor(SystemColor systemColor, Color4ub& outColor)
{
	const wxSystemColour c_systemColors[] =
	{
		wxSYS_COLOUR_BACKGROUND,
		wxSYS_COLOUR_ACTIVECAPTION,
		wxSYS_COLOUR_INACTIVECAPTION,
		wxSYS_COLOUR_MENU,
		wxSYS_COLOUR_WINDOW,
		wxSYS_COLOUR_WINDOWFRAME,
		wxSYS_COLOUR_MENUTEXT,
		wxSYS_COLOUR_WINDOWTEXT,
		wxSYS_COLOUR_CAPTIONTEXT,
		wxSYS_COLOUR_INACTIVECAPTIONTEXT,
		wxSYS_COLOUR_ACTIVEBORDER,
		wxSYS_COLOUR_INACTIVEBORDER,
		wxSYS_COLOUR_BTNFACE,
		wxSYS_COLOUR_BTNSHADOW,
		wxSYS_COLOUR_GRAYTEXT
	};

	wxColour colour = wxSystemSettings::GetColour(c_systemColors[systemColor]);
	outColor = Color4ub(
		colour.Red(),
		colour.Green(),
		colour.Blue(),
		255
	);

	return true;
}

void WidgetFactoryWx::getSystemFonts(std::list< std::wstring >& outFonts)
{
}

	}
}
