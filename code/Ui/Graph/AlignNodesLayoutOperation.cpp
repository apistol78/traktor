/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Graph/AlignNodesLayoutOperation.h"

#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/NodeMovedEvent.h"

#include <algorithm>
#include <limits>

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.AlignNodesLayoutOperation", AlignNodesLayoutOperation, IGraphLayoutOperation);

AlignNodesLayoutOperation::AlignNodesLayoutOperation(Alignment alignment)
:	m_alignment(alignment)
{
}

void AlignNodesLayoutOperation::apply(GraphControl* graph) const
{
	RefArray< Node > nodes = graph->getSelectedNodes();

	UnitRect bounds(
		Unit(std::numeric_limits< int32_t >::max()),
		Unit(std::numeric_limits< int32_t >::max()),
		Unit(-std::numeric_limits< int32_t >::max()),
		Unit(-std::numeric_limits< int32_t >::max()));
	for (auto node : nodes)
	{
		const UnitRect rc = node->calculateRect();
		bounds.left = std::min(bounds.left, rc.left);
		bounds.right = std::max(bounds.right, rc.right);
		bounds.top = std::min(bounds.top, rc.top);
		bounds.bottom = std::max(bounds.bottom, rc.bottom);
	}

	for (auto node : nodes)
	{
		const UnitRect rc = node->calculateRect();
		UnitPoint pt = rc.getTopLeft();

		switch (m_alignment)
		{
		case AnLeft:
			pt.x = bounds.left;
			break;

		case AnTop:
			pt.y = bounds.top;
			break;

		case AnRight:
			pt.x = bounds.right - rc.getWidth();
			break;

		case AnBottom:
			pt.y = bounds.bottom - rc.getHeight();
			break;
		}

		if (pt != node->getPosition())
		{
			node->setPosition(pt);
			NodeMovedEvent event(graph, node);
			graph->raiseEvent(&event);
		}
	}
}

}
