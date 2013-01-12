#include "Mesh/Editor/MeshUtilities.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"

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
		if ((*i)->getState().blendEnable)
			return false;
	}

	return true;
}

	}
}
