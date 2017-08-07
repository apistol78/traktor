/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ProgramCompilerCapture_H
#define traktor_render_ProgramCompilerCapture_H

#include "Render/IProgramCompiler.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_CAPTURE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief Wrapped program compiler.
 * \ingroup Render
 */
class T_DLLCLASS ProgramCompilerCapture : public IProgramCompiler
{
	T_RTTI_CLASS;

public:
	ProgramCompilerCapture(IProgramCompiler* compiler);

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

private:
	Ref< IProgramCompiler > m_compiler;
};

	}
}

#endif	// traktor_render_ProgramCompilerCapture_H
