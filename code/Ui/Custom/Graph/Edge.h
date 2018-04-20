/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_Edge_H
#define traktor_ui_custom_Edge_H

#include <vector>
#include "Core/Object.h"
#include "Ui/Associative.h"
#include "Ui/Point.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Size;

		namespace custom
		{

class GraphCanvas;
class PaintSettings;
class Pin;

/*! \brief Graph edge.
 * \ingroup UIC
 */
class T_DLLCLASS Edge
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	Edge(Pin* source, Pin* destination);

	void setSourcePin(Pin* source);

	Pin* getSourcePin() const;

	void setDestinationPin(Pin* destination);

	Pin* getDestinationPin() const;

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	void setSelected(bool selected);

	bool isSelected() const;

	bool hit(const PaintSettings* paintSettings, const Point& p) const;

	void paint(GraphCanvas* canvas, const Size& offset) const;

private:
	Ref< Pin > m_source;
	Ref< Pin > m_destination;
	std::wstring m_text;
	bool m_selected;
	std::map< std::wstring, Ref< Object > > m_data;
	mutable std::vector< Point > m_spline;
};

		}
	}
}

#endif	// traktor_ui_custom_Edge_H
