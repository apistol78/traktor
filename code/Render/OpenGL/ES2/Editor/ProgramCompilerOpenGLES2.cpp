/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/TString.h"
#include "Core/Serialization/DeepHash.h"
#include "Render/OpenGL/ES2/ProgramResourceOpenGLES2.h"
#include "Render/OpenGL/ES2/Editor/ProgramCompilerOpenGLES2.h"
#include "Render/OpenGL/ES2/Editor/Glsl/Glsl.h"
#include "Render/OpenGL/ES2/Editor/Glsl/GlslProgram.h"

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
	const std::wstring& name,
	int32_t optimize,
	bool validate,
	Stats* outStats
) const
{
	GlslProgram glslProgram;
	if (!Glsl().generate(shaderGraph, settings, name, glslProgram))
		return 0;

	Ref< ProgramResourceOpenGLES2 > resource = new ProgramResourceOpenGLES2(
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

bool ProgramCompilerOpenGLES2::generate(
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
