#ifndef traktor_mesh_MeshUtilities_H
#define traktor_mesh_MeshUtilities_H

namespace traktor
{
	namespace render
	{

class ShaderGraph;

	}

	namespace mesh
	{

bool isOpaqueMaterial(const render::ShaderGraph* shaderGraph);

	}
}

#endif	// traktor_mesh_MeshUtilities_H
