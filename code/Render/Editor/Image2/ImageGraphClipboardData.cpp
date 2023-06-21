/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/Image2/ImageGraphClipboardData.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageGraphClipboardData", 0, ImageGraphClipboardData, ISerializable)

void ImageGraphClipboardData::addNode(Node* node)
{
	m_nodes.push_back(node);
}

void ImageGraphClipboardData::addEdge(Edge* edge)
{
	m_edges.push_back(edge);
}

void ImageGraphClipboardData::setBounds(const ui::UnitRect& bounds)
{
	m_bounds = bounds;
}

const RefArray< Node >& ImageGraphClipboardData::getNodes() const
{
	return m_nodes;
}

const RefArray< Edge >& ImageGraphClipboardData::getEdges() const
{
	return m_edges;
}

const ui::UnitRect& ImageGraphClipboardData::getBounds() const
{
	return m_bounds;
}

void ImageGraphClipboardData::serialize(ISerializer& s)
{
	s >> MemberRefArray< Node >(L"nodes", m_nodes);
	s >> MemberRefArray< Edge >(L"edges", m_edges);
	//s >> Member< int32_t >(L"boundsLeft", m_bounds.left);
	//s >> Member< int32_t >(L"boundsTop", m_bounds.top);
	//s >> Member< int32_t >(L"boundsRight", m_bounds.right);
	//s >> Member< int32_t >(L"boundsBottom", m_bounds.bottom);
}

}
