#pragma once

#include "Core/IRefCount.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ProgramParameters;
class Shader;

	}

	namespace mesh
	{

/*! Set custom shader parameter callback.
 *
 * Called when meshes are rendered in order to allow
 * user defined shader parameters.
 */
class T_DLLCLASS IMeshParameterCallback : public IRefCount
{
public:
	virtual void setParameters(render::ProgramParameters* programParameters) const = 0;
};

	}
}

