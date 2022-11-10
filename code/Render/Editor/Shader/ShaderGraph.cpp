/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/ShaderGraph.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ShaderGraph", 0, ShaderGraph, Graph)

ShaderGraph::ShaderGraph()
:	Graph()
{
}

ShaderGraph::ShaderGraph(const RefArray< Node >& nodes, const RefArray< Edge >& edges)
:	Graph(nodes, edges)
{
}

	}
}
