/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Ui/Associative.h"
#include "Ui/Point.h"
#include "Ui/Size.h"
#include "Ui/Unit.h"

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
class Pin;

/*! Graph edge.
 * \ingroup UI
 */
class T_DLLCLASS Edge
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	Edge() = default;

	explicit Edge(Pin* source, Pin* destination);

	void setSourcePin(Pin* source);

	Pin* getSourcePin() const;

	void setDestinationPin(Pin* destination);

	Pin* getDestinationPin() const;

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	void setThickness(Unit thickness);

	Unit getThickness() const;

	void setSelected(bool selected);

	bool isSelected() const;

	bool hit(const GraphControl* graph, const UnitPoint& p) const;

	void paint(GraphControl* graph, GraphCanvas* canvas, const Size& offset, IBitmap* imageLabel, bool hot) const;

private:
	Ref< Pin > m_source;
	Ref< Pin > m_destination;
	std::wstring m_text;
	Unit m_thickness = Unit(2);
	bool m_selected = false;
	SmallMap< std::wstring, Ref< Object > > m_data;
	mutable AlignedVector< Point > m_spline;
};

}
