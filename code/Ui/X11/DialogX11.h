/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Itf/IDialog.h"
#include "Ui/X11/WidgetX11Impl.h"

namespace traktor::ui
{

class DialogX11 : public WidgetX11Impl< IDialog >
{
public:
	explicit DialogX11(Context* context, EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) override final;

	virtual void destroy() override final;

	virtual void setIcon(ISystemBitmap* icon) override final;

	virtual DialogResult showModal() override final;

	virtual void endModal(DialogResult result) override final;

	virtual void setMinSize(const Size& minSize) override final;

	virtual void setText(const std::wstring& text) override final;

	virtual void setVisible(bool visible) override final;

private:
	Atom m_atomWmDeleteWindow;
	Window m_parent;
	DialogResult m_result;
	bool m_modal;
};

}
