#include <stack>
#include "Render/Editor/Shader/ShaderGraphTechniques.h"
#include "Render/ShaderGraph.h"
#include "Render/ShaderGraphAdjacency.h"
#include "Render/Nodes.h"
#include "Render/Edge.h"
#include "Core/Serialization/DeepClone.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphTechniques", ShaderGraphTechniques, Object)

ShaderGraphTechniques::ShaderGraphTechniques(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
,	m_shaderGraphAdj(gc_new< ShaderGraphAdjacency >(shaderGraph))
{
}

std::set< std::wstring > ShaderGraphTechniques::getNames() const
{
	std::set< std::wstring > names;

	const RefArray< Node >& nodes = m_shaderGraph->getNodes();
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		if (VertexOutput* vertexOutput = dynamic_type_cast< VertexOutput* >(*i))
			names.insert(vertexOutput->getTechnique());
		else if (PixelOutput* pixelOutput = dynamic_type_cast< PixelOutput* >(*i))
			names.insert(pixelOutput->getTechnique());
	}

	return names;
}

ShaderGraph* ShaderGraphTechniques::generate(const std::wstring& name) const
{
	Ref< ShaderGraph > shaderGraph = gc_new< ShaderGraph >();
	std::stack< Node* > nodeStack;
	std::set< Node* > nodeVisited;

	// Get initial nodes.
	const RefArray< Node >& nodes = m_shaderGraph->getNodes();
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		if (VertexOutput* vertexOutput = dynamic_type_cast< VertexOutput* >(*i))
		{
			if (vertexOutput->getTechnique() == name)
				nodeStack.push(vertexOutput);
		}
		else if (PixelOutput* pixelOutput = dynamic_type_cast< PixelOutput* >(*i))
		{
			if (pixelOutput->getTechnique() == name)
				nodeStack.push(pixelOutput);
		}
	}

	// Traverse graph; copy branches which we can reach.
	while (!nodeStack.empty())
	{
		Ref< Node > node = nodeStack.top();
		nodeStack.pop();

		// Already visited this node?
		if (nodeVisited.find(node) != nodeVisited.end())
			continue;

		nodeVisited.insert(node);

		shaderGraph->addNode(node);

		int inputPinCount = node->getInputPinCount();
		for (int i = 0; i < inputPinCount; ++i)
		{
			Ref< const InputPin > inputPin = node->getInputPin(i);
			T_ASSERT (inputPin);

			Ref< Edge > edge = m_shaderGraphAdj->findEdge(inputPin);
			if (edge)
			{
				shaderGraph->addEdge(edge);
				nodeStack.push(edge->getSource()->getNode());
			}
		}
	}

	// Return a clone of combination graph as we don't want references between
	// graph instances.
	return DeepClone(shaderGraph).create< ShaderGraph >();
}

	}
}
