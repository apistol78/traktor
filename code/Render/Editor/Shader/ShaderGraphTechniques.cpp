/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <stack>
#include "Core/Serialization/DeepClone.h"
#include "Render/Editor/Shader/Edge.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphTraverse.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/ShaderGraphStatic.h"
#include "Render/Editor/Shader/ShaderGraphTechniques.h"

namespace traktor
{
	namespace render
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

ShaderGraphTechniques::ShaderGraphTechniques(const ShaderGraph* shaderGraph)
{
	Ref< ShaderGraph > shaderGraphOpt = ShaderGraphOptimizer(shaderGraph).removeUnusedBranches();

	// Constant fold entire graph so disabled outputs can be efficiently evaluated.
	if (shaderGraphOpt)
		shaderGraphOpt = ShaderGraphStatic(shaderGraphOpt).getConstantFolded();
	if (shaderGraphOpt)
		shaderGraphOpt = ShaderGraphStatic(shaderGraphOpt).removeDisabledOutputs();
	
	// Get all technique names.
	if (shaderGraphOpt)
	{
		std::set< std::wstring > names;

		const RefArray< Node >& nodes = shaderGraphOpt->getNodes();
		for (const auto node : nodes)
		{
			if (PixelOutput* pixelOutput = dynamic_type_cast< PixelOutput* >(node))
				names.insert(pixelOutput->getTechnique());
			else if (ComputeOutput* computeOutput = dynamic_type_cast< ComputeOutput* >(node))
				names.insert(computeOutput->getTechnique());
		}

		// Generate each technique.
		for (const auto& name : names)
		{
			RefArray< Node > roots;

			bool foundNamedVertexOutput = false;

			// Find named output nodes.
			for (RefArray< Node >::const_iterator j = nodes.begin(); j != nodes.end(); ++j)
			{
				if (VertexOutput* vertexOutput = dynamic_type_cast< VertexOutput* >(*j))
				{
					if (vertexOutput->getTechnique() == name)
					{
						roots.push_back(vertexOutput);
						foundNamedVertexOutput = true;
					}
				}
				else if (PixelOutput* pixelOutput = dynamic_type_cast< PixelOutput* >(*j))
				{
					if (pixelOutput->getTechnique() == name)
						roots.push_back(pixelOutput);
				}
				else if (ComputeOutput* computeOutput = dynamic_type_cast< ComputeOutput* >(*j))
				{
					if (computeOutput->getTechnique() == name)
						roots.push_back(computeOutput);
				}
				else
				{
					const INodeTraits* traits = INodeTraits::find(*j);
					T_FATAL_ASSERT (traits);

					if (traits->isRoot(shaderGraphOpt, *j))
						roots.push_back(*j);
				}
			}

			// If no explicit named vertex output we'll try to find an unnamed vertex output.
			if (!foundNamedVertexOutput)
			{
				for (RefArray< Node >::const_iterator j = nodes.begin(); j != nodes.end(); ++j)
				{
					VertexOutput* vertexOutput = dynamic_type_cast< VertexOutput* >(*j);
					if (vertexOutput && vertexOutput->getTechnique().empty())
						roots.push_back(vertexOutput);
				}
			}

			CopyVisitor visitor;
			visitor.m_shaderGraph = new ShaderGraph();
			ShaderGraphTraverse(shaderGraphOpt, roots).preorder(visitor);
			m_techniques[name] = visitor.m_shaderGraph;
		}
	}
}

std::set< std::wstring > ShaderGraphTechniques::getNames() const
{
	std::set< std::wstring > names;
	for (std::map< std::wstring, Ref< const ShaderGraph > >::const_iterator i = m_techniques.begin(); i != m_techniques.end(); ++i)
		names.insert(i->first);
	return names;
}

Ref< ShaderGraph > ShaderGraphTechniques::generate(const std::wstring& name) const
{
	std::map< std::wstring, Ref< const ShaderGraph > >::const_iterator i = m_techniques.find(name);
	return i != m_techniques.end() ? DeepClone(i->second).create< ShaderGraph >() : 0;
}

	}
}
