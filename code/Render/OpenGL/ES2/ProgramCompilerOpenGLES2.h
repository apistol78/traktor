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
	virtual const wchar_t* getPlatformSignature() const T_OVERRIDE T_FINAL;

	virtual Ref< ProgramResource > compile(
		const ShaderGraph* shaderGraph,
		const PropertyGroup* settings,
		int32_t optimize,
		bool validate,
		Stats* outStats
	) const T_OVERRIDE T_FINAL;

	virtual bool generate(
		const ShaderGraph* shaderGraph,
		const PropertyGroup* settings,
		int32_t optimize,
		std::wstring& outVertexShader,
		std::wstring& outPixelShader
	) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_render_ProgramCompilerOpenGLES2_H
