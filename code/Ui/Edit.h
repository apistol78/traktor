/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class EditValidator;

/*! Text edit.
 * \ingroup UI
 */
class T_DLLCLASS Edit : public Widget
{
	T_RTTI_CLASS;

public:
	constexpr static uint32_t WsReadOnly = WsUser;

	Edit();

	bool create(Widget* parent, const std::wstring& text = L"", uint32_t style = WsNone, const EditValidator* validator = nullptr);

	virtual void destroy() override;

	bool setValidator(const EditValidator* validator);

	const EditValidator* getValidator() const;

	void select(int from, int to);

	void selectAll();

	void deselect();

	bool haveSelection() const;

	std::wstring getSelectedText() const;

	void insert(const std::wstring& text);

	void copy();

	void cut();

	void paste();

	void setBorderColor(const Color4ub& borderColor);

	virtual void setText(const std::wstring& text) override;

	virtual Size getPreferredSize(const Size& hint) const override;

	virtual Size getMaximumSize() const override;

private:
	Ref< const EditValidator > m_validator;
	Color4ub m_borderColor;
	int32_t m_selectionStart;
	int32_t m_selectionEnd;
	int32_t m_caret;
	bool m_caretBlink;
	bool m_acceptTab;
	bool m_readOnly;
	bool m_hover;

	void eventFocus(FocusEvent* event);

	void eventMouseTrack(MouseTrackEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventDoubleClick(MouseDoubleClickEvent* event);

	void eventKeyDown(KeyDownEvent* event);

	void eventKey(KeyEvent* event);

	void eventPaint(PaintEvent* event);

	void eventTimer(TimerEvent* event);
};

}
