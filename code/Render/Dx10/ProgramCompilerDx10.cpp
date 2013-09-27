#include "Core/Log/Log.h"
#include "Render/Dx10/Hlsl.h"
#include "Render/Dx10/HlslProgram.h"
#include "Render/Dx10/ProgramCompilerDx10.h"
#include "Render/Dx10/ProgramDx10.h"
#include "Render/Dx10/ProgramResourceDx10.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerDx10", 0, ProgramCompilerDx10, IProgramCompiler)

const wchar_t* ProgramCompilerDx10::getPlatformSignature() const
{
	return L"DX10";
}

Ref< ProgramResource > ProgramCompilerDx10::compile(
	const ShaderGraph* shaderGraph,
	int32_t optimize,
	bool validate,
	IProgramHints* hints,
	Stats* outStats
) const
{
	// Generate HLSL shaders.
	HlslProgram hlslProgram;
	if (!Hlsl().generate(shaderGraph, hlslProgram))
		return 0;

	// Compile shaders.
	Ref< ProgramResource > programResource = ProgramDx10::compile(hlslProgram);
	if (!programResource)
		return 0;

	return programResource;
}

bool ProgramCompilerDx10::generate(
	const ShaderGraph* shaderGraph,
	int32_t optimize,
	std::wstring& outShader
) const
{
	HlslProgram hlslProgram;
	if (!Hlsl().generate(shaderGraph, hlslProgram))
		return false;

	outShader =
		std::wstring(L"// Vertex shader\n") +
		std::wstring(L"\n") +
		hlslProgram.getVertexShader() +
		std::wstring(L"\n") +
		std::wstring(L"// Pixel shader\n") +
		std::wstring(L"\n") +
		hlslProgram.getPixelShader();

	return true;
}

	}
}
