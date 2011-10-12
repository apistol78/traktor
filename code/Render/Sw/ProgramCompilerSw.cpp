#include "Core/Log/Log.h"
#include "Render/Sw/ProgramCompilerSw.h"
#include "Render/Sw/ProgramResourceSw.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerSw", 0, ProgramCompilerSw, IProgramCompiler)

const wchar_t* ProgramCompilerSw::getPlatformSignature() const
{
	return L"Software";
}

Ref< ProgramResource > ProgramCompilerSw::compile(
	const ShaderGraph* shaderGraph,
	int32_t optimize,
	bool validate,
	IProgramHints* hints,
	Stats* outStats
) const
{
	return new ProgramResourceSw(shaderGraph);
}

	}
}
