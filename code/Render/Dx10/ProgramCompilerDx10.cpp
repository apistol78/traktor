#include "Core/Log/Log.h"
#include "Render/Dx10/Hlsl.h"
#include "Render/Dx10/HlslProgram.h"
#include "Render/Dx10/ProgramCompilerDx10.h"
#include "Render/Dx10/ProgramDx10.h"
#include "Render/Dx10/ProgramResourceDx10.h"
#include "Render/Shader/ShaderGraphOptimizer.h"
#include "Render/Shader/ShaderGraphStatic.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerDx10", 0, ProgramCompilerDx10, IProgramCompiler)

Ref< ProgramResource > ProgramCompilerDx10::compile(
	const ShaderGraph* shaderGraph,
	int32_t optimize,
	bool validate,
	uint32_t* outCostEstimate
) const
{
	Ref< ShaderGraph > programGraph;

	// Extract platform permutation.
	programGraph = ShaderGraphStatic(shaderGraph).getPlatformPermutation(L"DX10");
	if (!programGraph)
	{
		log::error << L"ProgramCompilerDx10 failed; unable to get platform permutation" << Endl;
		return 0;
	}

	// Freeze type permutation.
	programGraph = ShaderGraphStatic(programGraph).getTypePermutation();
	if (!programGraph)
	{
		log::error << L"ProgramCompilerDx10 failed; unable to get type permutation" << Endl;
		return 0;
	}

	// Merge identical branches.
	programGraph = ShaderGraphOptimizer(programGraph).mergeBranches();
	if (!programGraph)
	{
		log::error << L"ProgramCompilerDx10 failed; unable to merge branches" << Endl;
		return 0;
	}

	// Insert interpolation nodes at optimal locations.
	programGraph = ShaderGraphOptimizer(programGraph).insertInterpolators();
	if (!programGraph)
	{
		log::error << L"ProgramCompilerDx10 failed; unable to optimize shader graph" << Endl;
		return 0;
	}

	// Generate HLSL shaders.
	HlslProgram hlslProgram;
	if (!Hlsl().generate(programGraph, hlslProgram))
		return 0;

	// Compile shaders.
	Ref< ProgramResource > programResource = ProgramDx10::compile(hlslProgram);
	if (!programResource)
		return 0;

	return programResource;
}

	}
}
