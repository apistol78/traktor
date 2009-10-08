#include "Ui/Cocoa/MenuBarCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/MenuItem.h"
#include "Ui/Itf/IForm.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{
		
NSMenu* createNSMenu(MenuItem* item)
{
	NSMenu* menu = [[NSMenu alloc] initWithTitle: makeNSString(item->getText())];
	
	for (int i = 0; i < item->count(); ++i)
	{
		Ref< MenuItem > subItem = item->get(i);
		
		NSMenuItem* menuItem = [[NSMenuItem alloc]
			initWithTitle: makeNSString(subItem->getText())
			action: nil
			keyEquivalent: @""
		];
		
		if (subItem->count() > 0)
		{
			NSMenu* subMenu = createNSMenu(subItem);
			[menuItem setSubmenu: subMenu];
		}
		
		[menu addItem: menuItem];
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
		NSMenu* subMenu = createNSMenu(item);
		[menuItem setSubmenu: subMenu];
	}
	
	[m_menu addItem: menuItem];
}
	
	}
}
