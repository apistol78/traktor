#include "Render/OpenGL/Glsl.h"
#include "Render/OpenGL/GlslProgram.h"
#include "Render/OpenGL/ES2/ProgramCompilerOpenGLES2.h"
#include "Render/OpenGL/ES2/ProgramOpenGLES2.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerOpenGLES2", 0, ProgramCompilerOpenGLES2, IProgramCompiler)

Ref< ProgramResource > ProgramCompilerOpenGLES2::compile(const ShaderGraph* shaderGraph, int32_t optimize, bool validate) const
{
	GlslProgram glslProgram;
	if (!Glsl().generate(shaderGraph, glslProgram))
		return 0;

	Ref< ProgramResource > resource = ProgramOpenGLES2::compile(glslProgram, optimize, validate);
	if (!resource)
		return 0;

	return resource;
}

	}
}
