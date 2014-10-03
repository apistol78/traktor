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

	}
}

#endif	// traktor_render_ProgramCompilerDx10_H
