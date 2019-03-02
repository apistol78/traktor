#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Render/OpenGL/Std/Editor/Glsl/Glsl.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslProgram.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslContext.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"

namespace traktor
{
	namespace render
	{

bool Glsl::generate(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	const std::wstring& name,
	GlslProgram& outProgram
)
{
	RefArray< VertexOutput > vertexOutputs;
	RefArray< PixelOutput > pixelOutputs;
	RefArray< ComputeOutput > computeOutputs;

	shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);
	shaderGraph->findNodesOf< ComputeOutput >(computeOutputs);

	if (!(
		(vertexOutputs.size() == 1 && pixelOutputs.size() == 1) ||
		computeOutputs.size() == 1
	))
	{
		log::error << L"Unable to generate GLSL shader; incorrect number of outputs (VS " << vertexOutputs.size() << L", PS " << pixelOutputs.size() << L", CS " << computeOutputs.size() << L")." << Endl;
		return false;
	}

	GlslContext cx(shaderGraph, settings);
	if (computeOutputs.empty())
	{
		if (!cx.emit(pixelOutputs[0]))
		{
			log::error << L"Unable to generate GLSL shader; emitter failed with fragment graph." << Endl;
			log::error << L"\t" << cx.getError() << Endl;
			return false;
		}
		if (!cx.emit(vertexOutputs[0]))
		{
			log::error << L"Unable to generate GLSL shader; emitter failed with vertex graph." << Endl;
			log::error << L"\t" << cx.getError() << Endl;
			return false;
		}
	}
	else
	{
		if (!cx.emit(computeOutputs[0]))
		{
			log::error << L"Unable to generate GLSL shader; emitter failed with compute graph." << Endl;
			log::error << L"\t" << cx.getError() << Endl;
			return false;
		}
	}

	GlslRequirements vertexRequirements = cx.requirements();
	vertexRequirements.derivatives = false;
	vertexRequirements.precisionHint = PhHigh;

	GlslRequirements fragmentRequirements = cx.requirements();

	GlslRequirements computeRequirements = cx.requirements();

	outProgram = GlslProgram(
		cx.getVertexShader().getGeneratedShader(settings, name, vertexRequirements),
		cx.getFragmentShader().getGeneratedShader(settings, name, fragmentRequirements),
		cx.getComputeShader().getGeneratedShader(settings, name, computeRequirements),
		cx.getTextures(),
		cx.getUniforms(),
		cx.getSamplers(),
		cx.getRenderState()
	);

	return true;
}

	}
}
