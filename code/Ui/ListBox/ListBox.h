/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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

namespace traktor
{
	namespace ui
	{

class ListBoxItem;

/*! List box control.
 * \ingroup UI
 */
class T_DLLCLASS ListBox : public AutoWidget
{
	T_RTTI_CLASS;

public:
	constexpr static uint32_t WsSingle = 0;
	constexpr static uint32_t WsMultiple = WsUser;
	constexpr static uint32_t WsExtended = (WsUser << 1);
	constexpr static uint32_t WsSort = (WsUser << 2);
	constexpr static uint32_t WsDefault = WsSingle;

	ListBox();

	bool create(Widget* parent, uint32_t style = WsDefault);

	virtual Size getPreferredSize(const Size& hint) const override;

	int32_t add(const std::wstring& item, Object* data = nullptr);

	int32_t add(const std::wstring& item, const Color4ub& bgcolor, Object* data = nullptr);

	bool remove(int32_t index);

	void removeAll();

	int32_t count() const;

	void setItem(int32_t index, const std::wstring& item);

	void setData(int32_t index, Object* data);

	std::wstring getItem(int32_t index) const;

	Object* getData(int32_t index) const;

	void select(int32_t index);

	bool selected(int32_t index) const;

	int32_t getSelected(std::vector< int32_t >& selected) const;

	int32_t getSelected() const;

	std::wstring getSelectedItem() const;

	Object* getSelectedData() const;

	int32_t getItemHeight();

	Rect getItemRect(int32_t index) const;

	template < typename T >
	T* getData(int index) const
	{
		return dynamic_type_cast< T* >(getData(index));
	}

	template < typename T >
	T* getSelectedData() const
	{
		return dynamic_type_cast< T* >(getSelectedData());
	}

private:
	friend class ListBoxItem;

	int32_t m_style;
	int32_t m_lastHitIndex;
	RefArray< ListBoxItem > m_items;

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventKeyDown(KeyDownEvent* event);

	virtual void layoutCells(const Rect& rc) override;
};

	}
}

