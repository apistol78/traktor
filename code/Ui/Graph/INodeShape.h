#pragma once

#include "Core/Object.h"
#include "Ui/Point.h"
#include "Ui/Size.h"

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
class Node;
class Pin;

/*! Graph node visual shape.
 * \ingroup UI
 */
class T_DLLCLASS INodeShape : public Object
{
	T_RTTI_CLASS;

public:
	virtual Point getPinPosition(const Node* node, const Pin* pin) const = 0;

	virtual Pin* getPinAt(const Node* node, const Point& pt) const = 0;

	virtual void paint(const Node* node, const Pin* hotPin, GraphCanvas* canvas, const Size& offset) const = 0;

	virtual Size calculateSize(const Node* node) const = 0;
};

	}
}

