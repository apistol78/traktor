/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

#endif

	}
}

#endif	// traktor_render_ProgramCompilerPs3_H
