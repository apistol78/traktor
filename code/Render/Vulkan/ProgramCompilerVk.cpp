#include "Render/Vulkan/ProgramCompilerVk.h"

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
	return L"Vulkan";
}

Ref< ProgramResource > ProgramCompilerVk::compile(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	int32_t optimize,
	bool validate,
	Stats* outStats
) const
{
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
	return false;
}

	}
}
