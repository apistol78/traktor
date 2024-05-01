/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Itf/IForm.h"
#include "Ui/X11/WidgetX11Impl.h"

namespace traktor::ui
{

class FormX11 : public WidgetX11Impl< IForm >
{
public:
	explicit FormX11(Context* context, EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) override final;

	virtual void setText(const std::wstring& text) override final;

	virtual void setIcon(ISystemBitmap* icon) override final;

	virtual void maximize() override final;

	virtual void minimize() override final;

	virtual void restore() override final;

	virtual bool isMaximized() const override final;

	virtual bool isMinimized() const override final;

	virtual void hideProgress() override final;

	virtual void showProgress(int32_t current, int32_t total) override final;

private:
	Atom m_atomWmDeleteWindow;
};

}

