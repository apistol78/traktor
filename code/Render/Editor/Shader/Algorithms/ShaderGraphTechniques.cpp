/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/ImmutableCheck.h"
#include "Core/Serialization/DeepClone.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/GraphTraverse.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Script.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphStatic.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphTechniques.h"

namespace traktor::render
{
	namespace
	{

struct CopyVisitor
{
	Ref< ShaderGraph > m_shaderGraph;

	bool operator () (Node* node)
	{
		m_shaderGraph->addNode(node);
		return true;
	}

	bool operator () (Edge* edge)
	{
		m_shaderGraph->addEdge(edge);
		return true;
	}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphTechniques", ShaderGraphTechniques, Object)

ShaderGraphTechniques::ShaderGraphTechniques(const ShaderGraph* shaderGraph, const Guid& shaderGraphId)
:	m_valid(true)
{
	T_IMMUTABLE_CHECK(shaderGraph);

	Ref< ShaderGraph > shaderGraphOpt = new ShaderGraph(
		shaderGraph->getNodes(),
		shaderGraph->getEdges()
	);

	// Constant fold entire graph so disabled outputs can be efficiently evaluated.
	if (shaderGraphOpt)
		shaderGraphOpt = ShaderGraphStatic(shaderGraphOpt, shaderGraphId).getConstantFolded();
	if (shaderGraphOpt)
		shaderGraphOpt = ShaderGraphStatic(shaderGraphOpt, shaderGraphId).removeDisabledOutputs();

	// Get all technique names.
	if (shaderGraphOpt)
	{
		std::set< std::wstring > names;

		const RefArray< Node >& nodes = shaderGraphOpt->getNodes();
		for (const auto node : nodes)
		{
			if (auto pixelOutput = dynamic_type_cast< PixelOutput* >(node))
				names.insert(pixelOutput->getTechnique());
			else if (auto computeOutput = dynamic_type_cast< ComputeOutput* >(node))
				names.insert(computeOutput->getTechnique());
			else if (auto script = dynamic_type_cast< Script* >(node))
			{
				if (!script->getTechnique().empty())
					names.insert(script->getTechnique());
			}
		}

		// Generate each technique.
		for (const auto& name : names)
		{
			RefArray< Node > roots;

			bool foundNamedVertexOutput = false;

			// Find named output nodes.
			for (auto node : nodes)
			{
				if (auto vertexOutput = dynamic_type_cast< VertexOutput* >(node))
				{
					if (vertexOutput->getTechnique() == name)
					{
						roots.push_back(vertexOutput);
						foundNamedVertexOutput = true;
					}
				}
				else if (auto pixelOutput = dynamic_type_cast< PixelOutput* >(node))
				{
					if (pixelOutput->getTechnique() == name)
						roots.push_back(pixelOutput);
				}
				else if (auto computeOutput = dynamic_type_cast< ComputeOutput* >(node))
				{
					if (computeOutput->getTechnique() == name)
						roots.push_back(computeOutput);
				}
				else if (auto script = dynamic_type_cast< Script* >(node))
				{
					if (script->getTechnique() == name)
						roots.push_back(script);
				}
				else
				{
					const INodeTraits* traits = INodeTraits::find(node);
					if (traits != nullptr && traits->isRoot(shaderGraphOpt, node))
						roots.push_back(node);
				}
			}

			// If no explicit named vertex output we'll try to find an unnamed vertex output.
			if (!foundNamedVertexOutput)
			{
				for (auto node : nodes)
				{
					VertexOutput* vertexOutput = dynamic_type_cast< VertexOutput* >(node);
					if (vertexOutput && vertexOutput->getTechnique().empty())
						roots.push_back(vertexOutput);
				}
			}

			CopyVisitor visitor;
			visitor.m_shaderGraph = new ShaderGraph();
			GraphTraverse(shaderGraphOpt, roots).preorder(visitor);
			m_techniques[name] = visitor.m_shaderGraph;
		}
	}
	else
		m_valid = false;
}

std::set< std::wstring > ShaderGraphTechniques::getNames() const
{
	std::set< std::wstring > names;
	for (auto technique : m_techniques)
		names.insert(technique.first);
	return names;
}

ShaderGraph* ShaderGraphTechniques::generate(const std::wstring& name) const
{
	auto it = m_techniques.find(name);
	if (it != m_techniques.end())
		return it->second;
	else
		return nullptr;
}

}
