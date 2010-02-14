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

#endif	// traktor_render_ProgramCompilerPs3_H
