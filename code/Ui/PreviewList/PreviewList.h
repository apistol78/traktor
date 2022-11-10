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

namespace traktor
{
	namespace ui
	{

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

	enum ItemSize
	{
		IsSmall = 0,
		IsLarge = 1
	};

	PreviewList();

	bool create(Widget* parent, uint32_t style);

	void setItemSize(ItemSize itemSize);

	void setItems(PreviewItems* items);

	Ref< PreviewItems > getItems() const;

	PreviewItem* getSelectedItem() const;

	void getSelectedItems(RefArray< PreviewItem >& outItems) const;

private:
	ItemSize m_itemSize;
	Ref< PreviewItems > m_items;
	bool m_single;

	virtual void layoutCells(const Rect& rc) override final;

	void eventButtonDown(MouseButtonDownEvent* event);
};

	}
}

