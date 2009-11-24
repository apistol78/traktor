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
	virtual Ref< ProgramResource > compile(const ShaderGraph* shaderGraph, int32_t optimize, bool validate) const;
};

	}
}

#endif	// traktor_render_ProgramCompilerOpenGLES2_H
