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
#include "Render/Editor/Shader/ShaderGraphEditorClipboardData.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderGraphEditorClipboardData", 0, ShaderGraphEditorClipboardData, ISerializable)

void ShaderGraphEditorClipboardData::addNode(Node* node)
{
	m_nodes.push_back(node);
}

void ShaderGraphEditorClipboardData::addEdge(Edge* edge)
{
	m_edges.push_back(edge);
}

void ShaderGraphEditorClipboardData::setBounds(const ui::Rect& bounds)
{
	m_bounds = bounds;
}

const RefArray< Node >& ShaderGraphEditorClipboardData::getNodes() const
{
	return m_nodes;
}

const RefArray< Edge >& ShaderGraphEditorClipboardData::getEdges() const
{
	return m_edges;
}

const ui::Rect& ShaderGraphEditorClipboardData::getBounds() const
{
	return m_bounds;
}

void ShaderGraphEditorClipboardData::serialize(ISerializer& s)
{
	s >> MemberRefArray< Node >(L"nodes", m_nodes);
	s >> MemberRefArray< Edge >(L"edges", m_edges);
	s >> Member< int32_t >(L"boundsLeft", m_bounds.left);
	s >> Member< int32_t >(L"boundsTop", m_bounds.top);
	s >> Member< int32_t >(L"boundsRight", m_bounds.right);
	s >> Member< int32_t >(L"boundsBottom", m_bounds.bottom);
}

	}
}
