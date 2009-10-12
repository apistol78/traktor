#include "Ui/Cocoa/MenuBarCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/MenuItem.h"
#include "Ui/EventSubject.h"
#include "Ui/Itf/IForm.h"
#include "Ui/Events/CommandEvent.h"
#include "Core/Log/Log.h"

@interface ObjCRef : NSObject
{
	traktor::Ref< traktor::Object >* m_ref;
}

- (id) initWithRef: (traktor::Object*)ptr;

- (void) set: (traktor::Object*)ptr;

- (traktor::Object*) get;

@end

@implementation ObjCRef

- (id) initWithRef: (traktor::Object*)ptr
{
	m_ref = new traktor::Ref< traktor::Object >(ptr);
	return self;
}

- (void) set: (traktor::Object*)ptr
{
	delete m_ref;
	m_ref = new traktor::Ref< traktor::Object >(ptr);
}

- (traktor::Object*) get
{
	return m_ref ? *m_ref : 0;
}

@end


namespace traktor
{
	namespace ui
	{
		namespace
		{
		
NSMenu* createNSMenu(ITargetProxyCallback* targetProxyCallback, MenuItem* item)
{
	NSMenu* menu = [[NSMenu alloc] initWithTitle: makeNSString(item->getText())];
	[menu setAutoenablesItems: NO];
	
	for (int i = 0; i < item->count(); ++i)
	{
		Ref< MenuItem > subItem = item->get(i);
		
		if (subItem->getText() != L"-")
		{
			NSTargetProxy* targetProxy = [[NSTargetProxy alloc] init];
			[targetProxy setCallback: targetProxyCallback];
			
			NSMenuItem* menuItem = [[NSMenuItem alloc]
				initWithTitle: makeNSString(subItem->getText())
				action: nil
				keyEquivalent: @""
			];
			
			[menuItem setTarget: targetProxy];
			[menuItem setAction: @selector(dispatchActionCallback:)];
			[menuItem setRepresentedObject: [[ObjCRef alloc] initWithRef: subItem]];
			
			if (subItem->count() > 0)
			{
				NSMenu* subMenu = createNSMenu(targetProxyCallback, subItem);
				[menuItem setSubmenu: subMenu];
			}
			
			[menu addItem: menuItem];
		}
		else
		{
			[menu addItem: [NSMenuItem separatorItem]];
		}
	}
	
	return menu;
}
		
		}
	
MenuBarCocoa::MenuBarCocoa(EventSubject* owner)
:	m_owner(owner)
,	m_menu(0)
{
}

bool MenuBarCocoa::create(IForm* form)
{
	m_menu = [[NSMenu alloc] initWithTitle: @""];
	[m_menu setAutoenablesItems: NO];
	
	[NSApp setMainMenu: m_menu];
	
	return true;
}

void MenuBarCocoa::destroy()
{
	[NSApp setMainMenu: NULL];
	[m_menu release];
}

void MenuBarCocoa::add(MenuItem* item)
{	
	NSMenuItem* menuItem = [[NSMenuItem alloc]
		initWithTitle: makeNSString(item->getText())
		action: nil
		keyEquivalent: @""
	];
	
	if (item->count())
	{
		NSMenu* subMenu = createNSMenu(this, item);
		[menuItem setSubmenu: subMenu];
	}
	
	[m_menu addItem: menuItem];
}

void MenuBarCocoa::targetProxy_Action(void* controlId)
{
	NSMenuItem* menuItem = (NSMenuItem*)controlId;
	ObjCRef* repsObj = (ObjCRef*)[menuItem representedObject];
	if (repsObj)
	{
		MenuItem* realMenuItem = dynamic_type_cast< MenuItem* >([repsObj get]);
		if (realMenuItem)
		{
			CommandEvent cmdEvent(m_owner, realMenuItem, realMenuItem->getCommand());
			m_owner->raiseEvent(EiClick, &cmdEvent);
		}
	}
}
	
	}
}
