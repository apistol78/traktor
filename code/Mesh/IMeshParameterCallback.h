#ifndef traktor_mesh_IMeshParameterCallback_H
#define traktor_mesh_IMeshParameterCallback_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ShaderParameters;

	}

	namespace mesh
	{

/*! \brief Set custom shader parameter callback.
 *
 * Called when meshes are rendered in order to allow
 * user defined shader parameters.
 */
class T_DLLCLASS IMeshParameterCallback
{
public:
	virtual void setParameters(render::ShaderParameters* shaderParameters) const = 0;
};

	}
}

#endif	// traktor_mesh_IMeshParameterCallback_H
