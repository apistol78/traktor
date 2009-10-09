#include "Ui/Cocoa/MenuBarCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/MenuItem.h"
#include "Ui/Itf/IForm.h"
#include "Core/Log/Log.h"

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
:	m_menu(0)
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

void MenuBarCocoa::targetProxy_Action()
{
	log::info << L"Menu item selected" << Endl;
}
	
	}
}
