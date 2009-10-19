#import "Ui/Cocoa/ObjCRef.h"

#include "Ui/Cocoa/PopupMenuCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/MenuItem.h"

namespace traktor
{
	namespace ui
	{
	
PopupMenuCocoa::PopupMenuCocoa(EventSubject* owner)
:	m_menu(0)
{
}

bool PopupMenuCocoa::create()
{
	m_menu = [[NSMenu alloc] initWithTitle: @""];
	[m_menu setAutoenablesItems: NO];

	return true;
}

void PopupMenuCocoa::destroy()
{
	[m_menu release];
}

void PopupMenuCocoa::add(MenuItem* item)
{
	if (item->getText() != L"-")
	{
		NSTargetProxy* targetProxy = [[NSTargetProxy alloc] init];
		[targetProxy setCallback: this];
	
		NSMenuItem* menuItem = [[NSMenuItem alloc]
			initWithTitle: makeNSString(item->getText())
			action: nil
			keyEquivalent: @""
		];
		[menuItem setTarget: targetProxy];
		[menuItem setAction: @selector(dispatchActionCallback:)];
		[menuItem setRepresentedObject: [[ObjCRef alloc] initWithRef: item]];
		
		[m_menu addItem: menuItem];
	}
	else
		[m_menu addItem: [NSMenuItem separatorItem]];
}

MenuItem* PopupMenuCocoa::show(IWidget* parent, const Point& at)
{
	NSView* view = (NSView*)parent->getInternalHandle();
	T_ASSERT (view);

	NSPoint nat = [view convertPoint: makeNSPoint(at) toView: nil];
	
	NSEvent* event = [NSEvent
		mouseEventWithType: NSLeftMouseDown
		location: nat
		modifierFlags: NSLeftMouseDownMask
		timestamp: 0
		windowNumber: [[view window] windowNumber]
		context: [[view window] graphicsContext]
		eventNumber: 0
		clickCount: 1
		pressure: 1
	];
	
	[NSMenu popUpContextMenu: m_menu withEvent: event forView: view];
	
	return m_selectedItem;
}

void PopupMenuCocoa::targetProxy_Action(void* controlId)
{
	NSMenuItem* menuItem = (NSMenuItem*)controlId;
	ObjCRef* repsObj = (ObjCRef*)[menuItem representedObject];
	if (repsObj)
		m_selectedItem = dynamic_type_cast< MenuItem* >([repsObj get]);
}

void PopupMenuCocoa::targetProxy_doubleAction(void* controlId)
{
}

	}
}
