#include "Render/Dx9/Hlsl.h"
#include "Render/Dx9/HlslProgram.h"
#include "Render/Dx9/ProgramResourceDx9.h"
#include "Render/Dx9/Win32/ProgramCompilerWin32.h"
#include "Render/Dx9/Win32/ProgramWin32.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerWin32", 0, ProgramCompilerWin32, IProgramCompiler)

Ref< ProgramResource > ProgramCompilerWin32::compile(const ShaderGraph* shaderGraph, int32_t optimize, bool validate) const
{
	HlslProgram program;
	if (!Hlsl().generate(shaderGraph, program))
		return 0;

	Ref< ProgramResourceDx9 > programResource = ProgramWin32::compile(program, optimize, validate);
	if (!programResource)
		return 0;

	return programResource;
}

	}
}
