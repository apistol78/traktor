/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_NodeShape_H
#define traktor_ui_custom_NodeShape_H

#include "Core/Object.h"
#include "Ui/Point.h"
#include "Ui/Size.h"

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
class Node;
class Pin;

/*! \brief Graph node visual shape.
 * \ingroup UIC
 */
class T_DLLCLASS NodeShape : public Object
{
	T_RTTI_CLASS;

public:
	virtual Point getPinPosition(const Node* node, const Pin* pin) const = 0;

	virtual Pin* getPinAt(const Node* node, const Point& pt) const = 0;

	virtual void paint(const Node* node, GraphCanvas* canvas, const Size& offset) const = 0;

	virtual Size calculateSize(const Node* node) const = 0;
};

		}
	}
}

#endif	// traktor_ui_custom_NodeShape_H
