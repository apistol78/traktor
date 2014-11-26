#ifndef traktor_render_ProgramCompilerPs3_H
#define traktor_render_ProgramCompilerPs3_H

#include "Render/IProgramCompiler.h"

namespace traktor
{
	namespace render
	{

#if !defined(_PS3)

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
		const PropertyGroup* settings,
		int32_t optimize,
		bool validate,
		Stats* outStats
	) const;

	virtual bool generate(
		const ShaderGraph* shaderGraph,
		const PropertyGroup* settings,
		int32_t optimize,
		std::wstring& outShader
	) const;
};

#endif

	}
}

#endif	// traktor_render_ProgramCompilerPs3_H
