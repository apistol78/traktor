/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Ui/Events/SelectionChangeEvent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class Edge;
class Group;
class Node;

/*! Node or edge selection event.
 * \ingroup UI
 */
class T_DLLCLASS SelectEvent : public SelectionChangeEvent
{
	T_RTTI_CLASS;

public:
	explicit SelectEvent(EventSubject* sender, const RefArray< Group >& groups, const RefArray< Node >& nodes, const RefArray< Edge >& edges);

	const RefArray< Group >& getGroups() const;

	const RefArray< Node >& getNodes() const;

	const RefArray< Edge >& getEdges() const;

private:
	RefArray< Group > m_groups;
	RefArray< Node > m_nodes;
	RefArray< Edge > m_edges;
};

}
