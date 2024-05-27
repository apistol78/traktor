/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Ui/Auto/AutoWidget.h"
#include "Ui/PreviewList/PreviewSelectionChangeEvent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class Edit;
class PreviewItems;

class T_DLLCLASS PreviewList : public ui::AutoWidget
{
	T_RTTI_CLASS;

public:
	enum
	{
		WsSingle = 0,
		WsMultiple = WsUser
	};

	bool create(Widget* parent, uint32_t style);

	void setItems(PreviewItems* items);

	Ref< PreviewItems > getItems() const;

	PreviewItem* getSelectedItem() const;

	void getSelectedItems(RefArray< PreviewItem >& outItems) const;

	void beginEdit(PreviewItem* item);

private:
	Ref< Edit > m_itemEditor;
	Ref< PreviewItems > m_items;
	bool m_single = true;
	int32_t m_dragMode = 0;
	Point m_dragOriginPosition;
	Ref< PreviewItem > m_editItem;

	virtual void layoutCells(const Rect& rc) override final;

	void eventEditFocus(FocusEvent* event);

	void eventEditKeyDownEvent(KeyDownEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);
};

}
