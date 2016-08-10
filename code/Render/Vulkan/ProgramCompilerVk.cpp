#include "Render/Vulkan/ProgramCompilerVk.h"
#include "Render/Vulkan/Glsl/Glsl.h"
#include "Render/Vulkan/Glsl/GlslProgram.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerVk", 0, ProgramCompilerVk, IProgramCompiler)

ProgramCompilerVk::ProgramCompilerVk()
{
}

const wchar_t* ProgramCompilerVk::getPlatformSignature() const
{
	return L"Vulkan GLSL";
}

Ref< ProgramResource > ProgramCompilerVk::compile(
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

	return 0;
}

bool ProgramCompilerVk::generate(
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
