/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Dialog.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class Button;
class ButtonClickEvent;

/*! Configuration dialog.
 * \ingroup UI
 */
class T_DLLCLASS ConfigDialog : public Dialog
{
	T_RTTI_CLASS;

public:
	constexpr static uint32_t WsOkCancelButtons = 0;
	constexpr static uint32_t WsYesNoButtons = (WsUser << 1);
	constexpr static uint32_t WsApplyButton = (WsUser << 2);

	bool create(Widget* parent, const std::wstring& text, Unit width, Unit height, uint32_t style, Layout* layout);

	virtual void destroy() override;

	virtual void update(const Rect* rc = nullptr, bool immediate = false) override;

	virtual Rect getInnerRect() const override;

	virtual Size getMinimumSize() const override;

private:
	Ref< Button > m_ok;
	Ref< Button > m_cancel;
	Ref< Button > m_apply;

	void eventButtonClick(ButtonClickEvent* event);
};

}
