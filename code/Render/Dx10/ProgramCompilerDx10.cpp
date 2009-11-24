#include "Render/Dx10/Hlsl.h"
#include "Render/Dx10/HlslProgram.h"
#include "Render/Dx10/ProgramCompilerDx10.h"
#include "Render/Dx10/ProgramResourceDx10.h"
#include "Render/Dx10/ProgramDx10.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerDx10", 0, ProgramCompilerDx10, IProgramCompiler)

Ref< ProgramResource > ProgramCompilerDx10::compile(const ShaderGraph* shaderGraph, int32_t optimize, bool validate) const
{
	HlslProgram hlslProgram;
	if (!Hlsl().generate(shaderGraph, hlslProgram))
		return 0;

	Ref< ProgramResource > programResource = ProgramDx10::compile(hlslProgram);
	if (!programResource)
		return 0;

	return programResource;
}

	}
}
