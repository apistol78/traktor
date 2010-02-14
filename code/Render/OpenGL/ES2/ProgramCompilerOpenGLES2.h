#ifndef traktor_render_ProgramCompilerOpenGLES2_H
#define traktor_render_ProgramCompilerOpenGLES2_H

#include "Render/IProgramCompiler.h"

namespace traktor
{
	namespace render
	{

/*! \brief OpenGL ES 2.0 program compiler.
 * \ingroup Render
 */
class ProgramCompilerOpenGLES2 : public IProgramCompiler
{
	T_RTTI_CLASS;

public:
	virtual const wchar_t* getPlatformSignature() const;

	virtual Ref< ProgramResource > compile(
		const ShaderGraph* shaderGraph,
		int32_t optimize,
		bool validate,
		uint32_t* outCostEstimate
	) const;
};

	}
}

#endif	// traktor_render_ProgramCompilerOpenGLES2_H
