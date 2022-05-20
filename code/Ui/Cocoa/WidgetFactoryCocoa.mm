#include "Ui/Cocoa/ClipboardCocoa.h"
#include "Ui/Cocoa/ContainerCocoa.h"
#include "Ui/Cocoa/DialogCocoa.h"
#include "Ui/Cocoa/EventLoopCocoa.h"
#include "Ui/Cocoa/FormCocoa.h"
#include "Ui/Cocoa/PathDialogCocoa.h"
#include "Ui/Cocoa/ToolFormCocoa.h"
#include "Ui/Cocoa/UserWidgetCocoa.h"
#include "Ui/Cocoa/BitmapCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Cocoa/WidgetFactoryCocoa.h"

namespace traktor
{
	namespace ui
	{

IEventLoop* WidgetFactoryCocoa::createEventLoop(EventSubject* owner)
{
	return new EventLoopCocoa();

}
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
	return nullptr;
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

void WidgetFactoryCocoa::getSystemFonts(std::list< std::wstring >& outFonts)
{
	NSArray* availableFontFamilies = [[NSFontManager sharedFontManager] availableFontFamilies];
	for (NSString* family in availableFontFamilies)
		outFonts.push_back(fromNSString(family));
}

void WidgetFactoryCocoa::getDesktopRects(std::list< Rect >& outRects) const
{
	NSRect frame = [[NSScreen mainScreen] frame];
	outRects.push_back(fromNSRect(frame));
}

	}
}
