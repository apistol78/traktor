#include <stack>
#include "Core/Serialization/DeepClone.h"
#include "Render/Shader/Edge.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Shader/ShaderGraphTraverse.h"
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

	// Get all technique names.
	std::set< std::wstring > names;

	const RefArray< Node >& nodes = shaderGraphOpt->getNodes();
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		if (VertexOutput* vertexOutput = dynamic_type_cast< VertexOutput* >(*i))
		{
			if (!vertexOutput->getTechnique().empty())
				names.insert(vertexOutput->getTechnique());
		}
		else if (PixelOutput* pixelOutput = dynamic_type_cast< PixelOutput* >(*i))
			names.insert(pixelOutput->getTechnique());
	}

	// Generate each technique.
	for (std::set< std::wstring >::const_iterator i = names.begin(); i != names.end(); ++i)
	{
		RefArray< Node > roots;

		bool foundNamedVertexOutput = false;

		// Find named output nodes.
		for (RefArray< Node >::const_iterator j = nodes.begin(); j != nodes.end(); ++j)
		{
			if (VertexOutput* vertexOutput = dynamic_type_cast< VertexOutput* >(*j))
			{
				if (vertexOutput->getTechnique() == *i)
				{
					roots.push_back(vertexOutput);
					foundNamedVertexOutput = true;
				}
			}
			else if (PixelOutput* pixelOutput = dynamic_type_cast< PixelOutput* >(*j))
			{
				if (pixelOutput->getTechnique() == *i)
					roots.push_back(pixelOutput);
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

		// Finally constant fold technique; this might return null which is fully valid as the
		// entire graph is collapsed to void, i.e doesn't produce any output, thus we ignore this technique.
		Ref< ShaderGraph > shaderGraphTechnique = ShaderGraphStatic(visitor.m_shaderGraph).getConstantFolded();
		if (shaderGraphTechnique)
			m_techniques[*i] = shaderGraphTechnique;
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
