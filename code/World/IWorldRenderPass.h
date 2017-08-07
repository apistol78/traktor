/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_world_IWorldRenderPass_H
#define traktor_world_IWorldRenderPass_H

#include "Core/Object.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Transform.h"
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
	enum PassFlag
	{
		PfNone = 0,
		PfFirst = 1 << 0,	//!< First pass for this frame.
		PfLast = 1 << 1		//!< Last pass for this frame.
	};

	virtual render::handle_t getTechnique() const = 0;

	/*! \brief Return flags of pass. */
	virtual uint32_t getPassFlags() const = 0;

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
	 * \param lastWorld Previous world transform.
	 * \param world World transform.
	 * \param bounds World bounds.
	 */
	virtual void setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world, const Aabb3& bounds) const = 0;
};
	
	}
}

#endif	// traktor_world_IWorldRenderPass_H
