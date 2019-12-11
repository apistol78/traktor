#pragma once

#include <map>
#include <vector>
#include "Core/Object.h"
#include "Ui/Associative.h"
#include "Ui/Point.h"

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

class GraphCanvas;
class IBitmap;
class PaintSettings;
class Pin;
class Size;

/*! Graph edge.
 * \ingroup UI
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

	void paint(GraphCanvas* canvas, const Size& offset, IBitmap* imageLabel) const;

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

