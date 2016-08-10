#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Vulkan/Glsl/Glsl.h"
#include "Render/Vulkan/Glsl/GlslProgram.h"
#include "Render/Vulkan/Glsl/GlslContext.h"

namespace traktor
{
	namespace render
	{

bool Glsl::generate(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	GlslProgram& outProgram
)
{
	RefArray< VertexOutput > vertexOutputs;
	RefArray< PixelOutput > pixelOutputs;

	shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);

	if (vertexOutputs.size() != 1 || pixelOutputs.size() != 1)
	{
		log::error << L"Unable to generate Vulkan GLSL shader; incorrect number of outputs" << Endl;
		return false;
	}

	GlslContext cx(shaderGraph);
	cx.getEmitter().emit(cx, pixelOutputs[0]);
	cx.getEmitter().emit(cx, vertexOutputs[0]);

	outProgram = GlslProgram(
		cx.getVertexShader().getGeneratedShader(),
		cx.getFragmentShader().getGeneratedShader()
	);

	return true;
}

	}
}
