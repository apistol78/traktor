#include "Core/Log/Log.h"
#include "Render/Shader/ShaderGraphOptimizer.h"
#include "Render/Shader/ShaderGraphStatic.h"
#include "Render/Sw/ProgramCompilerSw.h"
#include "Render/Sw/ProgramResourceSw.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerSw", 0, ProgramCompilerSw, IProgramCompiler)

Ref< ProgramResource > ProgramCompilerSw::compile(const ShaderGraph* shaderGraph, int32_t optimize, bool validate) const
{
	Ref< ShaderGraph > programGraph;

	// Extract platform permutation.
	programGraph = ShaderGraphStatic(shaderGraph).getPlatformPermutation(L"Software");
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

	return new ProgramResourceSw(programGraph);
}

	}
}
