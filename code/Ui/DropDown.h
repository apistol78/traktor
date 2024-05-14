/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
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

/*! Drop down control.
 * \ingroup UI
 */
class T_DLLCLASS DropDown : public Widget
{
	T_RTTI_CLASS;

public:
	constexpr static uint32_t WsSingle = 0;
	constexpr static uint32_t WsMultiple = WsUser;
	constexpr static uint32_t WsDefault = WsSingle;

	bool create(Widget* parent, uint32_t style = WsDefault);

	int32_t add(const std::wstring& item, Object* data = 0);

	bool remove(int32_t index);

	void removeAll();

	int32_t count() const;

	void setItem(int32_t index, const std::wstring& item);

	void setData(int32_t index, Object* data);

	std::wstring getItem(int32_t index) const;

	Object* getData(int32_t index) const;

	void select(int32_t index);

	bool select(const std::wstring& item);

	void unselect(int32_t index);

	bool selected(int32_t index) const;

	int32_t getSelected() const;

	int32_t getSelected(std::vector< int32_t >& selected) const;

	std::wstring getSelectedItem() const;

	Object* getSelectedData() const;

	template < typename T >
	T* getData(int32_t index) const
	{
		return dynamic_type_cast< T* >(getData(index));
	}

	template < typename T >
	T* getSelectedData() const
	{
		return dynamic_type_cast< T* >(getSelectedData());
	}

	virtual Size getPreferredSize(const Size& hint) const override;

	virtual Size getMaximumSize() const override;

private:
	struct Item
	{
		std::wstring text;
		Ref< Object > data;
		bool selected;
	};

	AlignedVector< Item > m_items;
	bool m_multiple = false;
	bool m_hover = false;

	void eventMouseTrack(MouseTrackEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventPaint(PaintEvent* event);
};

}
