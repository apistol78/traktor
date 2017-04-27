/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/Graph/SelectEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.SelectEvent", SelectEvent, SelectionChangeEvent)

SelectEvent::SelectEvent(EventSubject* sender, const RefArray< Node >& nodes, const RefArray< Edge >& edges)
:	SelectionChangeEvent(sender)
,	m_nodes(nodes)
,	m_edges(edges)
{
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
	}
}
