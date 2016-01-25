#include "Ui/Cocoa/WidgetFactoryCocoa.h"
#include "Ui/Cocoa/ButtonCocoa.h"
#include "Ui/Cocoa/CheckBoxCocoa.h"
#include "Ui/Cocoa/ClipboardCocoa.h"
#include "Ui/Cocoa/ContainerCocoa.h"
#include "Ui/Cocoa/DialogCocoa.h"
#include "Ui/Cocoa/DropDownCocoa.h"
#include "Ui/Cocoa/EditCocoa.h"
#include "Ui/Cocoa/FileDialogCocoa.h"
#include "Ui/Cocoa/FormCocoa.h"
#include "Ui/Cocoa/ListBoxCocoa.h"
#include "Ui/Cocoa/ListViewCocoa.h"
#include "Ui/Cocoa/MenuBarCocoa.h"
#include "Ui/Cocoa/MessageBoxCocoa.h"
#include "Ui/Cocoa/PathDialogCocoa.h"
#include "Ui/Cocoa/PopupMenuCocoa.h"
#include "Ui/Cocoa/ScrollBarCocoa.h"
#include "Ui/Cocoa/SliderCocoa.h"
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
	return new DialogCocoa(owner);
}

IDropDown* WidgetFactoryCocoa::createDropDown(EventSubject* owner)
{
	return new DropDownCocoa(owner);
}

IEdit* WidgetFactoryCocoa::createEdit(EventSubject* owner)
{
	return new EditCocoa(owner);
}

IFileDialog* WidgetFactoryCocoa::createFileDialog(EventSubject* owner)
{
	return new FileDialogCocoa(owner);
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
	return new ListViewCocoa(owner);
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

IPathDialog* WidgetFactoryCocoa::createPathDialog(EventSubject* owner)
{
	return new PathDialogCocoa(owner);
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
	return new SliderCocoa(owner);
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

IWebBrowser* WidgetFactoryCocoa::createWebBrowser(EventSubject* owner)
{
	return 0;
}

INative* WidgetFactoryCocoa::createNative(EventSubject* owner)
{
	return 0;
}

ISystemBitmap* WidgetFactoryCocoa::createBitmap()
{
	return new BitmapCocoa();
}

IClipboard* WidgetFactoryCocoa::createClipboard()
{
	return new ClipboardCocoa();
}

 int32_t WidgetFactoryCocoa::getSystemDPI() const
 {
 	return 96;
 }

bool WidgetFactoryCocoa::getSystemColor(SystemColor systemColor, Color4ub& outColor)
{
	NSColor* color;
	switch (systemColor)
	{
	case ScDesktopBackground:
		color = [NSColor windowBackgroundColor];
		break;
	case ScActiveCaption:
		{
			outColor = Color4ub(153, 180, 209);
			return true;
		}
		break;
	case ScInactiveCaption:
		{
			outColor = Color4ub(163, 190, 219);
			return true;
		}
		break;
	case ScMenuBackground:
		color = [NSColor textBackgroundColor];
		break;
	case ScWindowBackground:
		{
			outColor = Color4ub(241, 241, 241);
			return true;
		}
		break;
	case ScWindowFrame:
		color = [NSColor windowFrameColor];		
		break;
	case ScMenuText:
		color = [NSColor textColor];
		break;
	case ScWindowText:
		color = [NSColor textColor];
		break;
	case ScActiveCaptionText:
		{
			outColor = Color4ub(255, 255, 255);
			return true;
		}
		break;
	case ScInactiveCaptionText:
		{
			outColor = Color4ub(0, 0, 0);
			return true;
		}
		break;
	case ScActiveBorder:
		color = [NSColor windowFrameColor];
		break;
	case ScInactiveBorder:
		color = [NSColor windowFrameColor];
		break;
	case ScButtonFace:
		{
			outColor = Color4ub(200, 200, 200);
			return true;
		}
		break;
	case ScButtonShadow:
		color = [NSColor controlShadowColor];
		break;
	case ScDisabledText:
		color = [NSColor disabledControlTextColor];
		break;
	default:
		return false;
	}
	
	// Ensure color is in RGB color space.
	color = [color colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
	
	// Extract color components.
	float r = [color redComponent];
	float g = [color greenComponent];
	float b = [color blueComponent];
	
	outColor = Color4ub(
		uint8_t(r * 255),
		uint8_t(g * 255),
		uint8_t(b * 255)
	);
	return true;
}

void WidgetFactoryCocoa::getSystemFonts(std::list< std::wstring >& outFonts)
{
}

	}
}
