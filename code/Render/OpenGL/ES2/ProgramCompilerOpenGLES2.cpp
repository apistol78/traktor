/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/OpenGL/ES2/ProgramCompilerOpenGLES2.h"
#include "Render/OpenGL/ES2/ProgramOpenGLES2.h"
#include "Render/OpenGL/Glsl/Glsl.h"
#include "Render/OpenGL/Glsl/GlslProgram.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerOpenGLES2", 0, ProgramCompilerOpenGLES2, IProgramCompiler)

const wchar_t* ProgramCompilerOpenGLES2::getPlatformSignature() const
{
	return L"OpenGL ES2";
}

Ref< ProgramResource > ProgramCompilerOpenGLES2::compile(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	int32_t optimize,
	bool validate,
	Stats* outStats
) const
{
	GlslProgram glslProgram;
	if (!Glsl().generate(shaderGraph, settings, glslProgram))
		return 0;

	return ProgramOpenGLES2::compile(glslProgram, optimize, validate);
}

bool ProgramCompilerOpenGLES2::generate(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	int32_t optimize,
	std::wstring& outVertexShader,
	std::wstring& outPixelShader
) const
{
	GlslProgram glslProgram;
	if (!Glsl().generate(shaderGraph, settings, glslProgram))
		return false;

	outVertexShader = glslProgram.getVertexShader();
	outPixelShader = glslProgram.getFragmentShader();
	return true;
}

	}
}
