/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
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
class GridItem;
class GridColumn;
class GridHeader;
class GridRow;
class HierarchicalState;

/*! Grid view control.
 * \ingroup UI
 */
class T_DLLCLASS GridView : public AutoWidget
{
	T_RTTI_CLASS;

public:
	constexpr static uint32_t WsColumnHeader = WsUser;
	constexpr static uint32_t WsAutoEdit = WsUser << 1;
	constexpr static uint32_t WsMultiSelect = WsUser << 2;

	enum GetFlags
	{
		GfDefault = 0,
		GfDescendants = 1,
		GfExpandedOnly = 2,
		GfSelectedOnly = 4
	};

	enum SortMode
	{
		SmLexical = 0,
		SmNumerical = 1
	};

	typedef std::function< bool (const GridRow*, const GridRow*) > sort_fn_t;

	GridView();

	bool create(Widget* parent, uint32_t style);

	void addColumn(GridColumn* column);

	GridColumn* getColumn(uint32_t index) const;

	const RefArray< GridColumn >& getColumns() const;

	void setSortColumn(int32_t columnIndex, bool ascending, SortMode mode);

	void setSort(const sort_fn_t& sortFn);

	int32_t getColumnIndex(int32_t x) const;

	void addRow(GridRow* row);

	void removeRow(GridRow* row);

	void removeAllRows();

	GridRow* getRow(int32_t index);

	const RefArray< GridRow >& getRows() const;

	RefArray< GridRow > getRows(uint32_t flags) const;

	GridRow* getSelectedRow() const;

	void selectAll();

	void deselectAll();

	void setMultiSelect(bool multiSelect);

	void fitColumn(int32_t columnIndex);

	Ref< HierarchicalState > captureState() const;

	void applyState(const HierarchicalState* state);

private:
	friend class GridItem;
	friend class GridRow;

	Ref< GridHeader > m_header;
	RefArray< GridColumn > m_columns;
	RefArray< GridRow > m_rows;
	Ref< GridRow > m_clickRow;
	int32_t m_clickColumn;
	int32_t m_sortColumnIndex;
	bool m_sortAscending;
	SortMode m_sortMode;
	sort_fn_t m_sortFn;
	bool m_autoEdit;
	bool m_multiSelect;
	Ref< Edit > m_itemEditor;
	Ref< GridItem > m_editItem;
	SmallMap< std::wstring, Ref< IBitmap > > m_bitmaps;

	virtual void layoutCells(const Rect& rc) override final;

	IBitmap* getBitmap(const wchar_t* const name);

	void beginEdit(GridItem* item);

	void eventEditFocus(FocusEvent* event);

	void eventEditKey(KeyDownEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventDoubleClick(MouseDoubleClickEvent* event);
};

}
