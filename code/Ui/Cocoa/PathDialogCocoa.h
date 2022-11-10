/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#import <Cocoa/Cocoa.h>

#include "Ui/Itf/IPathDialog.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class PathDialogCocoa : public IPathDialog
{
public:
	PathDialogCocoa(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& title) override final;

	virtual void destroy() override final;

	virtual DialogResult showModal(Path& outPath) override final;

private:
	EventSubject* m_owner;
	NSOpenPanel* m_openPanel;
};

	}
}

