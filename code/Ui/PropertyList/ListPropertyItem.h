/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
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

class ListBox;
class MiniButton;
class ToolForm;

/*! List property item.
 * \ingroup UI
 */
class T_DLLCLASS ListPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	ListPropertyItem(const std::wstring& text);

	virtual ~ListPropertyItem();

	int add(const std::wstring& item);

	bool remove(int index);

	void removeAll();

	int count() const;

	std::wstring get(int index) const;

	void select(int index);

	int getSelected() const;

	std::wstring getSelectedItem() const;

protected:
	virtual void createInPlaceControls(PropertyList* parent) override;

	virtual void destroyInPlaceControls() override;

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects) override;

	virtual void paintValue(PropertyList* parent, Canvas& canvas, const Rect& rc) override;

private:
	std::vector< std::wstring > m_items;
	int32_t m_selected;
	Ref< MiniButton > m_buttonDrop;
	Ref< ToolForm > m_listForm;
	Ref< ListBox > m_listBox;
	Rect m_listRect;

	void eventDropClick(ButtonClickEvent* event);

	void eventSelect(SelectionChangeEvent* event);

	void eventFocus(FocusEvent* event);

	void eventKeyDown(KeyDownEvent* event);
};

	}
}

