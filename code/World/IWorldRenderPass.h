#ifndef traktor_world_IWorldRenderPass_H
#define traktor_world_IWorldRenderPass_H

#include "Core/Object.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/IntervalTransform.h"
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

	/*! \brief Return true if this is the first pass rendered from eye/camera point of view. */
	virtual bool isFirstPassFromEye() const = 0;

	/*! \brief Set shader technique used by this pass. */
	virtual void setShaderTechnique(render::Shader* shader) const = 0;

	/*! \brief Set shader combination used by this pass. */
	virtual void setShaderCombination(render::Shader* shader) const = 0;

	/*! \brief Set shader combination used by this pass and parameters. */
	virtual void setShaderCombination(render::Shader* shader, const Transform& world, const Aabb3& bounds) const = 0;

	/*! \brief Set shader parameters.
	 *
	 * \param programParams Block to set parameters into.
	 */
	virtual void setProgramParameters(render::ProgramParameters* programParams) const = 0;

	/*! \brief Set shader parameters.
	 *
	 * \param programParams Block to set parameters into.
	 * \param world World transform.
	 * \param bounds World bounds.
	 */
	virtual void setProgramParameters(render::ProgramParameters* programParams, const Transform& world, const Aabb3& bounds) const = 0;

	/*! \brief Set shader parameters.
	 *
	 * \param programParams Block to set parameters into.
	 * \param world World transform.
	 * \param bounds World bounds.
	 */
	virtual void setProgramParameters(render::ProgramParameters* programParams, const IntervalTransform& world, const Aabb3& bounds) const = 0;
};
	
	}
}

#endif	// traktor_world_IWorldRenderPass_H
