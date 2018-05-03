/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <stddef.h>
#include <shader/wave_psslc.h>
#include "Render/Ps4/ProgramCompilerPs4.h"
#include "Render/Ps4/Pssl/Pssl.h"
#include "Render/Ps4/Pssl/PsslProgram.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerPs4", 0, ProgramCompilerPs4, IProgramCompiler)

ProgramCompilerPs4::ProgramCompilerPs4()
{
}

const wchar_t* ProgramCompilerPs4::getPlatformSignature() const
{
	return L"GNM";
}

Ref< ProgramResource > ProgramCompilerPs4::compile(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	int32_t optimize,
	bool validate,
	Stats* outStats
) const
{
	PsslProgram psslProgram;
	if (!Pssl().generate(shaderGraph, psslProgram))
		return 0;

	return 0;
}

bool ProgramCompilerPs4::generate(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	int32_t optimize,
	std::wstring& outVertexShader,
	std::wstring& outPixelShader
) const
{
	PsslProgram psslProgram;
	if (!Pssl().generate(shaderGraph, psslProgram))
		return false;

	outVertexShader = psslProgram.getVertexShader();
	outPixelShader = psslProgram.getPixelShader();

	return true;
}

	}
}
