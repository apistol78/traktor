/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Ui/Associative.h"
#include "Ui/Rect.h"

 // import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif
	
namespace traktor::ui
{

class GraphCanvas;
class GraphControl;
class IBitmap;

/*!
 * \ingroup UI
 */
class T_DLLCLASS Group
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	void setTitle(const std::wstring& title);

	const std::wstring& getTitle() const;

	void setPosition(const Point& position);

	const Point& getPosition() const;

	void setSize(const Size& size);

	const Size& getSize() const;

	void setAnchorPosition(int32_t anchor, const Point& position);

	Point getAnchorPosition(int32_t anchor) const;

	Rect calculateRect() const;

	bool hit(const Point& p) const;

	bool hitTitle(const Point& p) const;

	int32_t hitAnchor(const Point& p) const;

	void setSelected(bool selected);

	bool isSelected() const;

	void paint(GraphCanvas* canvas, const Size& offset) const;

private:
	friend class GraphControl;

	GraphControl* m_owner = nullptr;
	Ref< IBitmap > m_image[2];
	std::wstring m_title;
	Point m_position;
	Size m_size;
	bool m_selected = false;

	explicit Group(GraphControl* owner, const std::wstring& title, const Point& position, const Size& size);
};

}
