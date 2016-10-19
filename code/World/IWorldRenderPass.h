#ifndef traktor_world_IWorldRenderPass_H
#define traktor_world_IWorldRenderPass_H

#include "Core/Object.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Matrix44.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
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

	namespace world
	{

/*! \brief World render pass.
 * \ingroup World
 */
class T_DLLCLASS IWorldRenderPass : public Object
{
	T_RTTI_CLASS;
	
public:
	virtual render::handle_t getTechnique() const = 0;

	virtual bool isFirstPassFromEye() const = 0;

	virtual void setShaderTechnique(render::Shader* shader) const = 0;

	virtual void setShaderCombination(render::Shader* shader) const = 0;

	virtual void setShaderCombination(render::Shader* shader, const Matrix44& world, const Aabb3& bounds) const = 0;

	virtual void setProgramParameters(render::ProgramParameters* programParams, uint32_t priority) const = 0;

	virtual void setProgramParameters(render::ProgramParameters* programParams, uint32_t priority, const Matrix44& world, const Aabb3& bounds) const = 0;
};
	
	}
}

#endif	// traktor_world_IWorldRenderPass_H
