#include "Core/Misc/TString.h"
#include "Core/Serialization/DeepHash.h"
#include "Render/OpenGL/ES/ProgramResourceOpenGLES.h"
#include "Render/OpenGL/ES/Editor/ProgramCompilerOpenGLES.h"
#include "Render/OpenGL/ES/Editor/Glsl/Glsl.h"
#include "Render/OpenGL/ES/Editor/Glsl/GlslProgram.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerOpenGLES", 0, ProgramCompilerOpenGLES, IProgramCompiler)

const wchar_t* ProgramCompilerOpenGLES::getRendererSignature() const
{
	return L"OpenGL ES";
}

Ref< ProgramResource > ProgramCompilerOpenGLES::compile(
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
		return nullptr;

	Ref< ProgramResourceOpenGLES > resource = new ProgramResourceOpenGLES(
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

bool ProgramCompilerOpenGLES::generate(
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
