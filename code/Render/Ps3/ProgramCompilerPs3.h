#ifndef traktor_render_ProgramCompilerPs3_H
#define traktor_render_ProgramCompilerPs3_H

#include "Render/IProgramCompiler.h"

namespace traktor
{
	namespace render
	{

class CgProgram;

/*! \brief PS3 program compiler.
 * \ingroup Render
 */
class ProgramCompilerPs3 : public IProgramCompiler
{
	T_RTTI_CLASS;

public:
	ProgramCompilerPs3();

	virtual ~ProgramCompilerPs3();

	virtual const wchar_t* getPlatformSignature() const;

	virtual Ref< ProgramResource > compile(
		const ShaderGraph* shaderGraph,
		int32_t optimize,
		bool validate,
		Stats* outStats
	) const;
};

	}
}

#endif	// traktor_render_ProgramCompilerPs3_H
