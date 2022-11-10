/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Point.h"
#include "Ui/PropertyList/PropertyItem.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Edit;
class MiniButton;

/*! Text property item.
 * \ingroup UI
 */
class T_DLLCLASS TextPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	TextPropertyItem(const std::wstring& text, const std::wstring& value, bool multiLine);

	void setValue(const std::wstring& value);

	const std::wstring& getValue() const;

protected:
	virtual void createInPlaceControls(PropertyList* parent) override;

	virtual void destroyInPlaceControls() override;

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects) override;

	virtual void mouseButtonDown(MouseButtonDownEvent* event) override;

	virtual void paintValue(Canvas& canvas, const Rect& rc) override;

	virtual bool copy() override;

	virtual bool paste() override;

private:
	Ref< Edit > m_editor;
	Ref< MiniButton > m_buttonEdit;
	std::wstring m_value;
	bool m_multiLine;

	void eventEditFocus(FocusEvent* event);

	void eventEditKeyDownEvent(KeyDownEvent* event);

	void eventClick(ButtonClickEvent* event);
};

	}
}

