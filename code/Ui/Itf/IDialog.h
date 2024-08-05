/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Itf/IWidget.h"

namespace traktor::ui
{

class ISystemBitmap;

/*! Dialog interface.
 * \ingroup UI
 */
class IDialog : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) = 0;

	virtual void setIcon(ISystemBitmap* icon) = 0;

	virtual DialogResult showModal() = 0;

	virtual void endModal(DialogResult result) = 0;

	virtual void setMinSize(const Size& minSize) = 0;
};

}
