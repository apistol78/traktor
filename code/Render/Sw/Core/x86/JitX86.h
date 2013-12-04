#ifndef traktor_render_JitX86_H
#define traktor_render_JitX86_H

#include "Render/Sw/Core/Processor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_SW_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief Just-in-time processor.
 * \ingroup SW
 *
 * This processor will translate intermediate
 * shader program into a CPU native program.
 * Thus resulting in very fast execution
 * of shader programs.
 */
class T_DLLCLASS JitX86 : public Processor
{
	T_RTTI_CLASS;

public:
	JitX86();

	virtual image_t compile(const IntrProgram& program) const;

	virtual void destroy(image_t image) const;

	virtual bool execute(
		const image_t image,
		int32_t instance,
		const Vector4* inUniforms,
		const Vector4* inVaryings,
		const Vector4& targetSize,
		const Vector4& fragmentPosition,
		const Ref< AbstractSampler >* inSamplers,
		Vector4* outVaryings
	) const;
};

	}
}

#endif	// traktor_render_JitX86_H
