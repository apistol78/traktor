#include "Render/OpenGL/Glsl.h"
#include "Render/OpenGL/GlslProgram.h"
#include "Render/OpenGL/Std/ProgramCompilerOpenGL.h"
#include "Render/OpenGL/Std/ProgramOpenGL.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerOpenGL", 0, ProgramCompilerOpenGL, IProgramCompiler)

Ref< ProgramResource > ProgramCompilerOpenGL::compile(const ShaderGraph* shaderGraph, int32_t optimize, bool validate) const
{
	GlslProgram glslProgram;
	if (!Glsl().generate(shaderGraph, glslProgram))
		return 0;

	Ref< ProgramResource > resource = ProgramOpenGL::compile(glslProgram, optimize, validate);
	if (!resource)
		return 0;

	return resource;
}

	}
}
