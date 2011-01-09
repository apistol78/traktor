#include <stack>
#include "Render/Shader/Edge.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphTechniques.h"
#include "Render/Editor/Shader/ShaderGraphUtilities.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct CopyVisitor
{
	Ref< ShaderGraph > m_shaderGraph;

	void operator () (Node* node) {
		m_shaderGraph->addNode(node);
	}

	void operator () (Edge* edge) {
		m_shaderGraph->addEdge(edge);
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphTechniques", ShaderGraphTechniques, Object)

ShaderGraphTechniques::ShaderGraphTechniques(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
{
}

std::set< std::wstring > ShaderGraphTechniques::getNames() const
{
	std::set< std::wstring > names;

	const RefArray< Node >& nodes = m_shaderGraph->getNodes();
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

	return names;
}

Ref< ShaderGraph > ShaderGraphTechniques::generate(const std::wstring& name) const
{
	RefArray< Node > roots;

	const RefArray< Node >& nodes = m_shaderGraph->getNodes();
	bool foundNamedVertexOutput = false;
	
	// Find named output nodes.
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		if (VertexOutput* vertexOutput = dynamic_type_cast< VertexOutput* >(*i))
		{
			if (vertexOutput->getTechnique() == name)
			{
				roots.push_back(vertexOutput);
				foundNamedVertexOutput = true;
			}
		}
		else if (PixelOutput* pixelOutput = dynamic_type_cast< PixelOutput* >(*i))
		{
			if (pixelOutput->getTechnique() == name)
				roots.push_back(pixelOutput);
		}
	}

	// If no explicit named vertex output we'll try to find an unnamed vertex output.
	if (!foundNamedVertexOutput)
	{
		for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			VertexOutput* vertexOutput = dynamic_type_cast< VertexOutput* >(*i);
			if (vertexOutput && vertexOutput->getTechnique().empty())
				roots.push_back(vertexOutput);
		}
	}

	CopyVisitor visitor;
	visitor.m_shaderGraph = new ShaderGraph();
	shaderGraphTraverse(m_shaderGraph, roots, visitor);

	return visitor.m_shaderGraph;
}

	}
}
