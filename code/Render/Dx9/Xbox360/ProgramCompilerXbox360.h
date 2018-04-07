/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ProgramCompilerXbox360_H
#define traktor_render_ProgramCompilerXbox360_H

#include "Render/IProgramCompiler.h"

namespace traktor
{
	namespace render
	{

/*! \brief Xbox360 program compiler.
 * \ingroup Render
 */
class ProgramCompilerXbox360 : public IProgramCompiler
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
		std::wstring& outVertexShader,
		std::wstring& outPixelShader
	) const;
};

	}
}

#endif	// traktor_render_ProgramCompilerXbox360_H
