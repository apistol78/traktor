/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Graph/INodeShape.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class IBitmap;

/*! Comment node shape.
 * \ingroup UI
 */
class T_DLLCLASS CommentNodeShape : public INodeShape
{
	T_RTTI_CLASS;

public:
	CommentNodeShape();

	virtual UnitPoint getPinPosition(GraphControl* graph, const Node* node, const Pin* pin) const override final;

	virtual Pin* getPinAt(GraphControl* graph, const Node* node, const UnitPoint& pt) const override final;

	virtual void paint(GraphControl* graph, const Node* node, GraphCanvas* canvas, const Pin* hotPin, const Size& offset) const override final;

	virtual UnitSize calculateSize(GraphControl* graph, const Node* node) const override final;

private:
	Ref< IBitmap > m_imageNode;
};

}
