#include <wave_psslc.h>
#include "Render/Ps4/Pssl.h"
#include "Render/Ps4/PsslProgram.h"
#include "Render/Ps4/ProgramCompilerPs4.h"

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
	std::wstring& outShader
) const
{
	PsslProgram psslProgram;
	if (!Pssl().generate(shaderGraph, psslProgram))
		return false;

	outShader =
		std::wstring(L"// Vertex shader\n") +
		std::wstring(L"\n") +
		psslProgram.getVertexShader() +
		std::wstring(L"\n") +
		std::wstring(L"// Pixel shader\n") +
		std::wstring(L"\n") +
		psslProgram.getPixelShader();

	return true;
}

	}
}
