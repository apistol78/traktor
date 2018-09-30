/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/TString.h"
#include "Core/Serialization/DeepHash.h"
#include "Render/OpenGL/Std/ProgramResourceOpenGL.h"
#include "Render/OpenGL/Std/Editor/Glsl/Glsl.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslProgram.h"
#include "Render/OpenGL/Std/Editor/ProgramCompilerOpenGL.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerOpenGL", 0, ProgramCompilerOpenGL, IProgramCompiler)

const wchar_t* ProgramCompilerOpenGL::getPlatformSignature() const
{
	return L"OpenGL";
}

Ref< ProgramResource > ProgramCompilerOpenGL::compile(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	const std::wstring& name,
	int32_t optimize,
	bool validate,
	Stats* outStats
) const
{
	GlslProgram glslProgram;
	if (!Glsl().generate(shaderGraph, settings, name, glslProgram))
		return 0;

	Ref< ProgramResourceOpenGL > resource = new ProgramResourceOpenGL(
		wstombs(glslProgram.getVertexShader()),
		wstombs(glslProgram.getFragmentShader()),
		glslProgram.getTextures(),
		glslProgram.getUniforms(),
		glslProgram.getSamplers(),
		glslProgram.getRenderState()
	);

	uint32_t hash = DeepHash(resource).get();
	resource->setHash(hash);

	return resource;
}

bool ProgramCompilerOpenGL::generate(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	const std::wstring& name,
	int32_t optimize,
	std::wstring& outVertexShader,
	std::wstring& outPixelShader,
	std::wstring& outComputeShader
) const
{
	GlslProgram glslProgram;
	if (!Glsl().generate(shaderGraph, settings, name, glslProgram))
		return false;

	outVertexShader = glslProgram.getVertexShader();
	outPixelShader = glslProgram.getFragmentShader();
	return true;
}

	}
}
