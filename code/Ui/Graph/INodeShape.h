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
class GraphControl;
class Node;
class Pin;

/*! Graph node visual shape.
 * \ingroup UI
 */
class T_DLLCLASS INodeShape : public Object
{
	T_RTTI_CLASS;

public:
	virtual Point getPinPosition(GraphControl* graph, const Node* node, const Pin* pin) const = 0;

	virtual Pin* getPinAt(GraphControl* graph, const Node* node, const Point& pt) const = 0;

	virtual void paint(GraphControl* graph, const Node* node, GraphCanvas* canvas, const Pin* hotPin, const Size& offset) const = 0;

	virtual Size calculateSize(GraphControl* graph, const Node* node) const = 0;
};

	}
}

