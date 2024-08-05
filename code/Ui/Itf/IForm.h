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

class EventSubject;
class ISystemBitmap;

/*! Form interface.
 * \ingroup UI
 */
class IForm : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) = 0;

	virtual void setIcon(ISystemBitmap* icon) = 0;

	virtual void maximize() = 0;

	virtual void minimize() = 0;

	virtual void restore() = 0;

	virtual bool isMaximized() const = 0;

	virtual bool isMinimized() const = 0;

	virtual void hideProgress() = 0;

	virtual void showProgress(int32_t current, int32_t total) = 0;
};

}
