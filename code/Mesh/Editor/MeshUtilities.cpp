#include "Mesh/Editor/MeshUtilities.h"
#include "Render/Nodes.h"
#include "Render/ShaderGraph.h"

namespace traktor
{
	namespace mesh
	{

bool isOpaqueMaterial(const render::ShaderGraph* shaderGraph)
{
	RefArray< render::PixelOutput > nodes;
	if (shaderGraph->findNodesOf< render::PixelOutput >(nodes) == 0)
		return true;

	for (RefArray< render::PixelOutput >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		if ((*i)->getBlendEnable())
			return false;
	}

	return true;
}

	}
}
