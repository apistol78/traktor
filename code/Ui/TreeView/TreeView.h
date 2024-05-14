/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Ui/Auto/AutoWidget.h"

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
class HierarchicalState;
class TreeViewItem;

/*! Tree view control.
 * \ingroup UI
 */
class T_DLLCLASS TreeView : public AutoWidget
{
	T_RTTI_CLASS;

public:
	constexpr static uint32_t WsAutoEdit = WsUser;
	constexpr static uint32_t WsDrag = WsUser << 1;
	constexpr static uint32_t WsTreeButtons = WsUser << 2;
	constexpr static uint32_t WsTreeLines = WsUser << 3;
	constexpr static uint32_t WsDefault = WsAutoEdit | WsTreeButtons | WsTreeLines;

	enum GetFlags
	{
		GfDefault = 0,
		GfDescendants = 1,
		GfExpandedOnly = 2,
		GfSelectedOnly = 4
	};

	bool create(Widget* parent, uint32_t style);

	int32_t addImage(IBitmap* image);

	bool setImage(int32_t imageIndex, IBitmap* image);

	void removeAllImages();

	Ref< TreeViewItem > createItem(TreeViewItem* parent, const std::wstring& text, int32_t imageColumns);

	void removeItem(TreeViewItem* item);

	void removeAllItems();

	uint32_t getItems(RefArray< TreeViewItem >& outItems, uint32_t flags) const;

	void deselectAll();

	Ref< HierarchicalState > captureState() const;

	void applyState(const HierarchicalState* state);

private:
	friend class TreeViewItem;

	RefArray< TreeViewItem > m_roots;
	Ref< IBitmap > m_imageState;
	RefArray< IBitmap > m_images;
	Font m_font;
	Font m_fontBold;
	Ref< Edit > m_itemEditor;
	Ref< TreeViewItem > m_editItem;
	bool m_autoEdit = false;

	int32_t getMaxImageHeight() const;

	virtual void layoutCells(const Rect& rc) override;

	void beginEdit(TreeViewItem* item);

	void eventEditFocus(FocusEvent* event);

	void eventEditKeyDownEvent(KeyDownEvent* event);

	void eventScroll(ScrollEvent* event);

	void eventKeyDown(KeyDownEvent* event);
};

}
