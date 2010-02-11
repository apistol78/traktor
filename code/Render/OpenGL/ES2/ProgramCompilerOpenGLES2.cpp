#include "Core/Log/Log.h"
#include "Render/OpenGL/Glsl.h"
#include "Render/OpenGL/GlslProgram.h"
#include "Render/OpenGL/ES2/ProgramCompilerOpenGLES2.h"
#include "Render/OpenGL/ES2/ProgramOpenGLES2.h"
#include "Render/Shader/ShaderGraphOptimizer.h"
#include "Render/Shader/ShaderGraphStatic.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerOpenGLES2", 0, ProgramCompilerOpenGLES2, IProgramCompiler)

Ref< ProgramResource > ProgramCompilerOpenGLES2::compile(const ShaderGraph* shaderGraph, int32_t optimize, bool validate) const
{
	Ref< ShaderGraph > programGraph;

	// Extract platform permutation.
	programGraph = ShaderGraphStatic(shaderGraph).getPlatformPermutation(L"OpenGL ES2");
	if (!programGraph)
	{
		log::error << L"ProgramCompilerOpenGL failed; unable to get platform permutation" << Endl;
		return 0;
	}

	// Merge identical branches.
	programGraph = ShaderGraphOptimizer(programGraph).mergeBranches();
	if (!programGraph)
	{
		log::error << L"ProgramCompilerOpenGL failed; unable to merge branches" << Endl;
		return 0;
	}

	// Insert interpolation nodes at optimal locations.
	programGraph = ShaderGraphOptimizer(programGraph).insertInterpolators();
	if (!programGraph)
	{
		log::error << L"ProgramCompilerOpenGL failed; unable to optimize shader graph" << Endl;
		return 0;
	}

	// Generate GLSL shader.
	GlslProgram glslProgram;
	if (!Glsl().generate(programGraph, glslProgram))
		return 0;

	Ref< ProgramResource > resource = ProgramOpenGLES2::compile(glslProgram, optimize, validate);
	if (!resource)
		return 0;

	return resource;
}

	}
}
