#ifndef traktor_render_ProgramCompilerDx10_H
#define traktor_render_ProgramCompilerDx10_H

#include "Render/IProgramCompiler.h"

namespace traktor
{
	namespace render
	{

/*! \brief DX10 program compiler.
 * \ingroup Render
 */
class ProgramCompilerDx10 : public IProgramCompiler
{
	T_RTTI_CLASS;

public:
	virtual Ref< ProgramResource > compile(const ShaderGraph* shaderGraph, int32_t optimize, bool validate) const;
};

	}
}

#endif	// traktor_render_ProgramCompilerDx10_H
