#ifndef traktor_render_ProgramCompilerSw_H
#define traktor_render_ProgramCompilerSw_H

#include "Render/IProgramCompiler.h"

namespace traktor
{
	namespace render
	{

/*! \brief Software program compiler.
 * \ingroup Render
 */
class ProgramCompilerSw : public IProgramCompiler
{
	T_RTTI_CLASS;

public:
	virtual const wchar_t* getPlatformSignature() const;

	virtual Ref< ProgramResource > compile(
		const ShaderGraph* shaderGraph,
		int32_t optimize,
		bool validate,
		IProgramHints* hints,
		Stats* outStats
	) const;
};

	}
}

#endif	// traktor_render_ProgramCompilerSw_H
