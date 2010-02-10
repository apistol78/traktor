#ifndef traktor_render_ProgramCompilerXbox360_H
#define traktor_render_ProgramCompilerXbox360_H

#include "Render/IProgramCompiler.h"

namespace traktor
{
	namespace render
	{

/*! \brief Xbox360 program compiler.
 * \ingroup Render
 */
class ProgramCompilerXbox360 : public IProgramCompiler
{
	T_RTTI_CLASS;

public:
	virtual Ref< ProgramResource > compile(const ShaderGraph* shaderGraph, int32_t optimize, bool validate) const;
};

	}
}

#endif	// traktor_render_ProgramCompilerXbox360_H
