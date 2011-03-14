#include "Core/Log/Log.h"
#include "Render/Dx11/Hlsl.h"
#include "Render/Dx11/HlslProgram.h"
#include "Render/Dx11/ProgramCompilerDx11.h"
#include "Render/Dx11/ProgramDx11.h"
#include "Render/Dx11/ProgramResourceDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerDx11", 0, ProgramCompilerDx11, IProgramCompiler)

const wchar_t* ProgramCompilerDx11::getPlatformSignature() const
{
	return L"DX11";
}

Ref< ProgramResource > ProgramCompilerDx11::compile(
	const ShaderGraph* shaderGraph,
	int32_t optimize,
	bool validate,
	Stats* outStats
) const
{
	// Generate HLSL shaders.
	HlslProgram hlslProgram;
	if (!Hlsl().generate(shaderGraph, hlslProgram))
		return 0;

	// Compile shaders.
	Ref< ProgramResource > programResource = ProgramDx11::compile(hlslProgram);
	if (!programResource)
		return 0;

	return programResource;
}

	}
}
