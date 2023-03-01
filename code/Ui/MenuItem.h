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
#include <vector>
#include "Core/RefArray.h"
#include "Ui/Associative.h"
#include "Ui/Command.h"
#include "Ui/Size.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class Canvas;
class IBitmap;
class MouseButtonDownEvent;
class MouseButtonUpEvent;
class MouseMoveEvent;
class Rect;
class Widget;

/*! Menu item.
 * \ingroup UI
 */
class T_DLLCLASS MenuItem
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	MenuItem(const Command& command, const std::wstring& text, bool checkBox, IBitmap* image);

	MenuItem(const std::wstring& text, bool checkBox, IBitmap* image);

	MenuItem(const Command& command, const std::wstring& text, IBitmap* image);

	MenuItem(const std::wstring& text, IBitmap* image);

	MenuItem(const Command& command, const std::wstring& text);

	MenuItem(const std::wstring& text);

	void setCommand(const Command& command);

	const Command& getCommand() const;

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	void setCheckBox(bool checkBox);

	bool getCheckBox() const;

	void setImage(IBitmap* image);

	IBitmap* getImage() const;

	void setEnable(bool enable);

	bool isEnable() const;

	void setChecked(bool checked);

	bool isChecked() const;

	void add(MenuItem* item);

	void remove(MenuItem* item);

	void removeAll();

	int count() const;

	Ref< MenuItem > get(int index);

	virtual Size getSize(const Widget* shell) const;

	virtual void paint(const Widget* shell, Canvas& canvas, const Rect& rc, bool tracking) const;

private:
	Command m_command;
	std::wstring m_text;
	bool m_checkBox;
	Ref< IBitmap > m_imageUnchecked;
	Ref< IBitmap > m_imageChecked;
	Ref< IBitmap > m_image;
	bool m_enable;
	bool m_checked;
	RefArray< MenuItem > m_items;
};

}
