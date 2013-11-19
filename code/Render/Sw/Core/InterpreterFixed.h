#ifndef traktor_render_InterpreterFixed_H
#define traktor_render_InterpreterFixed_H

#include "Render/Sw/Core/Processor.h"
#include "Render/Sw/Core/Types.h"
#include "Render/Sw/Core/Fixed.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_SW_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief Interpreting processor.
 * \ingroup SW
 *
 * Interpreting processor implemented using
 * fixed-point arithmetic.
 */
class T_DLLCLASS InterpreterFixed : public Processor
{
	T_RTTI_CLASS;

public:
	InterpreterFixed();

	virtual image_t compile(const IntrProgram& program) const;

	virtual void destroy(image_t image) const;

	virtual void execute(
		const image_t image,
		int32_t instance,
		const Vector4* inUniforms,
		const Vector4* inVaryings,
		const Vector4& targetSize,
		const Ref< AbstractSampler >* inSamplers,
		Vector4* outVaryings
	) const;

private:
	static fp32_t ms_trig[256];
};

	}
}

#endif	// traktor_render_InterpreterFixed_H
