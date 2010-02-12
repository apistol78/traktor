#ifndef traktor_render_ProgramCompilerOpenGL_H
#define traktor_render_ProgramCompilerOpenGL_H

#include "Render/IProgramCompiler.h"

namespace traktor
{
	namespace render
	{

/*! \brief OpenGL program compiler.
 * \ingroup Render
 */
class ProgramCompilerOpenGL : public IProgramCompiler
{
	T_RTTI_CLASS;

public:
	virtual Ref< ProgramResource > compile(
		const ShaderGraph* shaderGraph,
		int32_t optimize,
		bool validate,
		uint32_t* outCostEstimate
	) const;
};

	}
}

#endif	// traktor_render_ProgramCompilerOpenGL_H
