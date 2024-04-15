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

namespace traktor
{

class Guid;

}

namespace traktor::ui
{

class HierarchicalState;
class PropertyItem;
class ScrollBar;

/*! Property list control.
 * \ingroup UI
 */
class T_DLLCLASS PropertyList : public Widget
{
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsColumnHeader = WsUser
	};

	enum GetPropertyItemFlags
	{
		GfDefault = 0,
		GfDescendants = 1,
		GfSelectedOnly = 2,
		GfExpandedOnly = 4,
		GfVisibleOnly = 8
	};

	struct IPropertyGuidResolver
	{
		virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const = 0;
	};

	PropertyList();

	bool create(Widget* parent, int style = WsDoubleBuffer, IPropertyGuidResolver* guidResolver = 0);

	virtual void destroy() override;

	void addPropertyItem(PropertyItem* propertyItem);

	void removePropertyItem(PropertyItem* propertyItem);

	void addPropertyItem(PropertyItem* parentPropertyItem, PropertyItem* propertyItem);

	void removePropertyItem(PropertyItem* parentPropertyItem, PropertyItem* propertyItem);

	void removeAllPropertyItems();

	int getPropertyItems(RefArray< PropertyItem >& propertyItems, int flags);

	void setSeparator(Unit separator);

	Unit getSeparator() const;

	void setColumnName(int column, const std::wstring& name);

	Ref< PropertyItem > getPropertyItemFromPosition(const Point& position);

	bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const;

	Ref< HierarchicalState > captureState() const;

	void applyState(const HierarchicalState* state);

	void expandAll();

	void collapseAll();

	virtual bool copy();

	virtual bool paste();

	virtual void update(const Rect* rc = nullptr, bool immediate = false) override;

	virtual Size getMinimumSize() const override;

	virtual Size getPreferredSize(const Size& hint) const override;

	IBitmap* getBitmap(const wchar_t* const name);

private:
	friend class PropertyItem;

	IPropertyGuidResolver* m_guidResolver;
	Ref< ScrollBar > m_scrollBar;
	RefArray< PropertyItem > m_propertyItems;
	Ref< PropertyItem > m_mousePropertyItem;
	Unit m_propertyItemHeight;
	Unit m_separator;
	int32_t m_mode;
	bool m_columnHeader;
	std::wstring m_columnNames[2];
	SmallMap< std::wstring, Ref< IBitmap > > m_bitmaps;

	void updateScrollBar();

	void placeItems();

	void eventScroll(ScrollEvent* event);

	void eventKeyDown(KeyDownEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventDoubleClick(MouseDoubleClickEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventMouseWheel(MouseWheelEvent* event);

	void eventSize(SizeEvent* event);

	void eventPaint(PaintEvent* event);
};

}
