#include "Core/Log/Log.h"
#include "Render/OpenGL/Glsl.h"
#include "Render/OpenGL/GlslProgram.h"
#include "Render/OpenGL/Std/ProgramCompilerOpenGL.h"
#include "Render/OpenGL/Std/ProgramOpenGL.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerOpenGL", 0, ProgramCompilerOpenGL, IProgramCompiler)

const wchar_t* ProgramCompilerOpenGL::getPlatformSignature() const
{
	return L"OpenGL";
}

Ref< ProgramResource > ProgramCompilerOpenGL::compile(
	const ShaderGraph* shaderGraph,
	int32_t optimize,
	bool validate,
	IProgramHints* hints,
	Stats* outStats
) const
{
	GlslProgram glslProgram;
	if (!Glsl().generate(shaderGraph, glslProgram))
		return 0;

	Ref< ProgramResource > resource = ProgramOpenGL::compile(glslProgram, optimize, validate);
	if (!resource)
		return 0;

	return resource;
}

bool ProgramCompilerOpenGL::generate(
	const ShaderGraph* shaderGraph,
	int32_t optimize,
	std::wstring& outShader
) const
{
	GlslProgram glslProgram;
	if (!Glsl().generate(shaderGraph, glslProgram))
		return false;

	outShader =
		std::wstring(L"// Vertex shader\n") +
		std::wstring(L"\n") +
		glslProgram.getVertexShader() +
		std::wstring(L"\n") +
		std::wstring(L"// Fragment shader\n") +
		std::wstring(L"\n") +
		glslProgram.getFragmentShader();

	return true;
}

	}
}
