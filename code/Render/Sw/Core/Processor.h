#ifndef traktor_render_Processor_H
#define traktor_render_Processor_H

#include "Core/Object.h"
#include "Core/Math/Vector4.h"

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

class IntrProgram;
class AbstractSampler;

/*! \brief Shader processor base class.
 * \ingroup SW
 */
class T_DLLCLASS Processor : public Object
{
	T_RTTI_CLASS;

public:
	typedef void* image_t;

	virtual image_t compile(const IntrProgram& program) const = 0;

	virtual void destroy(image_t image) const = 0;

	virtual bool execute(
		const image_t image,
		int32_t instance,
		const Vector4* inUniforms,
		const Vector4* inVaryings,
		const Vector4& targetSize,
		const Vector4& fragmentPosition,
		const Ref< AbstractSampler >* inSamplers,
		Vector4* outVaryings
	) const = 0;
};

	}
}

#endif	// traktor_render_Processor_H
