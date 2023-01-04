/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"
#include "Ui/Rect.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Node;
class Edge;

/*! Image graph clipboard daa container.
 * \ingroup Render
 */
class T_DLLCLASS ImageGraphClipboardData : public ISerializable
{
	T_RTTI_CLASS;

public:
	void addNode(Node* node);

	void addEdge(Edge* edge);

	void setBounds(const ui::Rect& bounds);

	const RefArray< Node >& getNodes() const;

	const RefArray< Edge >& getEdges() const;

	const ui::Rect& getBounds() const;

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< Node > m_nodes;
	RefArray< Edge > m_edges;
	ui::Rect m_bounds;
};

}
