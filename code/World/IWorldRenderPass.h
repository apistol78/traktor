#pragma once

#include "Core/Object.h"
#include "Core/Math/Transform.h"
#include "Render/Shader.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class IProgram;
class ProgramParameters;

}

namespace traktor::world
{

/*! World render pass.
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

	/*! Get shader technique. */
	virtual render::handle_t getTechnique() const = 0;

	/*! Return flags of pass. */
	virtual uint32_t getPassFlags() const = 0;

	/*! Get shader permutation. */
	virtual render::Shader::Permutation getPermutation(const render::Shader* shader) const = 0;

	/*! Set shader parameters.
	 *
	 * \param programParams Block to set parameters into.
	 */
	virtual void setProgramParameters(render::ProgramParameters* programParams) const = 0;

	/*! Set shader parameters.
	 *
	 * \param programParams Block to set parameters into.
	 * \param lastWorld Previous world transform.
	 * \param world World transform.
	 */
	virtual void setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world) const = 0;

	/*! Get shader program. */
	render::Shader::Program getProgram(const render::Shader* shader) const
	{
		auto perm = getPermutation(shader);
		return shader->getProgram(perm);
	}
};

}
