/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Graph/EvenSpaceLayoutOperation.h"

#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/NodeMovedEvent.h"

#include <algorithm>
#include <limits>

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.EvenSpaceLayoutOperation", EvenSpaceLayoutOperation, IGraphLayoutOperation);

EvenSpaceLayoutOperation::EvenSpaceLayoutOperation(EvenSpace space)
:	m_space(space)
{
}

void EvenSpaceLayoutOperation::apply(GraphControl* graph) const
{
	RefArray< Node > nodes = graph->getSelectedNodes();

	if (nodes.size() <= 1)
		return;

	const EvenSpace space = m_space;
	nodes.sort([space](const Node* n1, const Node* n2) {
		const UnitPoint pt1 = n1->calculateRect().getTopLeft();
		const UnitPoint pt2 = n2->calculateRect().getTopLeft();
		return space == EsHorizontally ? pt1.x < pt2.x : pt1.y < pt2.y;
	});

	UnitRect bounds(
		Unit(std::numeric_limits< int32_t >::max()),
		Unit(std::numeric_limits< int32_t >::max()),
		Unit(-std::numeric_limits< int32_t >::max()),
		Unit(-std::numeric_limits< int32_t >::max()));

	Unit totalWidth = 0_ut, totalHeight = 0_ut;

	for (auto node : nodes)
	{
		const UnitRect rc = node->calculateRect();

		bounds.left = std::min(bounds.left, rc.left);
		bounds.right = std::max(bounds.right, rc.right);
		bounds.top = std::min(bounds.top, rc.top);
		bounds.bottom = std::max(bounds.bottom, rc.bottom);

		totalWidth += rc.getWidth();
		totalHeight += rc.getHeight();
	}

	const Unit spaceHoriz = (bounds.getWidth() - totalWidth) / Unit((int32_t)nodes.size() - 1);
	const Unit spaceVert = (bounds.getHeight() - totalHeight) / Unit((int32_t)nodes.size() - 1);

	Unit x = bounds.left, y = bounds.top;

	for (auto node : nodes)
	{
		const UnitRect rc = node->calculateRect();
		UnitPoint pt = rc.getTopLeft();

		switch (m_space)
		{
		case EsHorizontally:
			pt.x = x;
			break;

		case EsVertically:
			pt.y = y;
			break;
		}

		if (pt != node->getPosition())
		{
			node->setPosition(pt);
			NodeMovedEvent event(graph, node);
			graph->raiseEvent(&event);
		}

		x += rc.getWidth() + spaceHoriz;
		y += rc.getHeight() + spaceVert;
	}
}

}
