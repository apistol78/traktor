/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Cocoa/ContainerCocoa.h"

namespace traktor
{
	namespace ui
	{

ContainerCocoa::ContainerCocoa(EventSubject* owner)
:	WidgetCocoaImpl< IContainer, NSCustomControl >(owner)
{
}

void ContainerCocoa::destroy()
{
	if (m_control)
		[m_control setCallback: nil];

	class_t::destroy();
}

bool ContainerCocoa::create(IWidget* parent, int style)
{
	m_control = [[NSCustomControl alloc]
		initWithFrame: NSMakeRect(0, 0, 0, 0)
	];
	[m_control setCallback: this];

	NSView* contentView = (__bridge NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);

	[contentView addSubview: m_control];

	return class_t::internalCreate();
}

bool ContainerCocoa::event_drawRect(const NSRect& rect)
{
	return false;
}

bool ContainerCocoa::event_viewDidEndLiveResize()
{
	raiseSizeEvent();
	return true;
}

bool ContainerCocoa::event_mouseDown(NSEvent* theEvent, int button)
{
	return false;
}

bool ContainerCocoa::event_mouseUp(NSEvent* theEvent, int button)
{
	return false;
}

bool ContainerCocoa::event_mouseMoved(NSEvent* theEvent, int button)
{
	return false;
}

bool ContainerCocoa::event_keyDown(NSEvent* theEvent)
{
	return false;
}

bool ContainerCocoa::event_keyUp(NSEvent* theEvent)
{
	return false;
}

bool ContainerCocoa::event_performKeyEquivalent(NSEvent* theEvent)
{
	return false;
}

	}
}
