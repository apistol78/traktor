#include "Render/Sw/ProgramCompilerSw.h"
#include "Render/Sw/ProgramResourceSw.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerSw", 0, ProgramCompilerSw, IProgramCompiler)

Ref< ProgramResource > ProgramCompilerSw::compile(const ShaderGraph* shaderGraph, int32_t optimize, bool validate) const
{
	return new ProgramResourceSw(shaderGraph);
}

	}
}
