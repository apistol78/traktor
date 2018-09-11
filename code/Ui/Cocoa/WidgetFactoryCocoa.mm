#include "Ui/Cocoa/WidgetFactoryCocoa.h"
#include "Ui/Cocoa/ClipboardCocoa.h"
#include "Ui/Cocoa/ContainerCocoa.h"
#include "Ui/Cocoa/DialogCocoa.h"
#include "Ui/Cocoa/FormCocoa.h"
#include "Ui/Cocoa/PathDialogCocoa.h"
#include "Ui/Cocoa/ToolFormCocoa.h"
#include "Ui/Cocoa/UserWidgetCocoa.h"
#include "Ui/Cocoa/BitmapCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"

namespace traktor
{
	namespace ui
	{

IContainer* WidgetFactoryCocoa::createContainer(EventSubject* owner)
{
	return new ContainerCocoa(owner);
}

IDialog* WidgetFactoryCocoa::createDialog(EventSubject* owner)
{
	return new DialogCocoa(owner);
}

IForm* WidgetFactoryCocoa::createForm(EventSubject* owner)
{
	return new FormCocoa(owner);
}

INotificationIcon* WidgetFactoryCocoa::createNotificationIcon(EventSubject* owner)
{
	return 0;
}

IPathDialog* WidgetFactoryCocoa::createPathDialog(EventSubject* owner)
{
	return new PathDialogCocoa(owner);
}

IToolForm* WidgetFactoryCocoa::createToolForm(EventSubject* owner)
{
	return new ToolFormCocoa(owner);
}

IUserWidget* WidgetFactoryCocoa::createUserWidget(EventSubject* owner)
{
	return new UserWidgetCocoa(owner);
}

IWebBrowser* WidgetFactoryCocoa::createWebBrowser(EventSubject* owner)
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
	NSArray* availableFontFamilies = [[NSFontManager sharedFontManager] availableFontFamilies];
	for (NSString* family in availableFontFamilies)
		outFonts.push_back(fromNSString(family));
}

	}
}
