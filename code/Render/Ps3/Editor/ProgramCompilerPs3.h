/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ProgramCompilerPs3_H
#define traktor_render_ProgramCompilerPs3_H

#include "Render/Editor/IProgramCompiler.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_PS3_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class CgProgram;

/*! \brief PS3 program compiler.
 * \ingroup Render
 */
class T_DLLCLASS ProgramCompilerPs3 : public IProgramCompiler
{
	T_RTTI_CLASS;

public:
	ProgramCompilerPs3();

	virtual ~ProgramCompilerPs3();

	virtual const wchar_t* getPlatformSignature() const T_OVERRIDE T_FINAL;

	virtual Ref< ProgramResource > compile(
		const ShaderGraph* shaderGraph,
		const PropertyGroup* settings,
		const std::wstring& name,
		int32_t optimize,
		bool validate,
		Stats* outStats
	) const T_OVERRIDE T_FINAL;

	virtual bool generate(
		const ShaderGraph* shaderGraph,
		const PropertyGroup* settings,
		const std::wstring& name,
		int32_t optimize,
		std::wstring& outVertexShader,
		std::wstring& outPixelShader,
		std::wstring& outComputeShader
	) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_render_ProgramCompilerPs3_H
