/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Graph/SelectEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.SelectEvent", SelectEvent, SelectionChangeEvent)

SelectEvent::SelectEvent(EventSubject* sender, const RefArray< Group >& groups, const RefArray< Node >& nodes, const RefArray< Edge >& edges)
:	SelectionChangeEvent(sender)
,	m_groups(groups)
,	m_nodes(nodes)
,	m_edges(edges)
{
}

const RefArray< Group >& SelectEvent::getGroups() const
{
	return m_groups;
}

const RefArray< Node >& SelectEvent::getNodes() const
{
	return m_nodes;
}

const RefArray< Edge >& SelectEvent::getEdges() const
{
	return m_edges;
}

}
