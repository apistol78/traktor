/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Cocoa/NSCustomControl.h"
#include "Ui/Itf/IUserWidget.h"

namespace traktor
{
	namespace ui
	{

class UserWidgetCocoa
:	public WidgetCocoaImpl< IUserWidget, NSCustomControl >
,	public INSControlEventsCallback
{
public:
	explicit UserWidgetCocoa(EventSubject* owner);

	// IWidget

	virtual void destroy() override final;

	// IUserWidget

	virtual bool create(IWidget* parent, int style) override final;

	// INSControlEventsCallback

	virtual bool event_drawRect(const NSRect& rect) override final;

	virtual bool event_viewDidEndLiveResize() override final;

	virtual bool event_mouseDown(NSEvent* theEvent, int button) override final;

	virtual bool event_mouseUp(NSEvent* theEvent, int button) override final;

	virtual bool event_mouseMoved(NSEvent* theEvent, int button) override final;

	virtual bool event_keyDown(NSEvent* theEvent) override final;

	virtual bool event_keyUp(NSEvent* theEvent) override final;

	virtual bool event_performKeyEquivalent(NSEvent* theEvent) override final;
};

	}
}

