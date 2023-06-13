/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Command.h"
#include "Ui/Point.h"
#include "Ui/Unit.h"
#include "Ui/ToolBar/ToolBarItem.h"

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

/*! Tool bar dropdown.
 * \ingroup UI
 */
class T_DLLCLASS ToolBarDropDown : public ToolBarItem
{
	T_RTTI_CLASS;

public:
	explicit ToolBarDropDown(const Command& command, Unit width, const std::wstring& toolTip);

	int32_t add(const std::wstring& item, Object* data = nullptr);

	bool remove(int32_t index);

	void removeAll();

	int32_t count() const;

	std::wstring getItem(int32_t index) const;

	Object* getData(int32_t index) const;

	void select(int32_t index);

	int32_t getSelected() const;

	std::wstring getSelectedItem() const;

	Object* getSelectedData() const;

protected:
	virtual bool getToolTip(std::wstring& outToolTip) const override final;

	virtual Size getSize(const ToolBar* toolBar, int imageWidth, int imageHeight) const override final;

	virtual void paint(ToolBar* toolBar, Canvas& canvas, const Point& at, IBitmap* images, int imageWidth, int imageHeight) override final;

	virtual bool mouseEnter(ToolBar* toolBar) override final;

	virtual void mouseLeave(ToolBar* toolBar) override final;

	virtual void buttonDown(ToolBar* toolBar, MouseButtonDownEvent* mouseEvent) override final;

	virtual void buttonUp(ToolBar* toolBar, MouseButtonUpEvent* mouseEvent) override final;

private:
	struct Item
	{
		std::wstring text;
		Ref< Object > data;
	};

	Command m_command;
	Unit m_width;
	std::wstring m_toolTip;
	std::vector< Item > m_items;
	int32_t m_selected;
	bool m_hover;
	int32_t m_dropPosition;
	Point m_menuPosition;
	int32_t m_menuWidth;
};

	}
}

