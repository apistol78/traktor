#include "Render/OpenGL/Glsl.h"
#include "Render/OpenGL/GlslProgram.h"
#include "Render/OpenGL/GlslContext.h"
#include "Render/ShaderGraph.h"
#include "Render/Nodes.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

bool Glsl::generate(
	const ShaderGraph* shaderGraph,
	GlslProgram& outProgram
)
{
	RefArray< VertexOutput > vertexOutputs;
	RefArray< PixelOutput > pixelOutputs;

	shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);

	if (vertexOutputs.size() != 1 || pixelOutputs.size() != 1)
	{
		log::error << L"Unable to generate GLSL shader; incorrect number of outputs" << Endl;
		return false;
	}

	GlslContext cx(shaderGraph);
	cx.getEmitter().emit(cx, pixelOutputs[0]);
	cx.getEmitter().emit(cx, vertexOutputs[0]);

	outProgram = GlslProgram(
		cx.getVertexShader().getGeneratedShader(),
		cx.getFragmentShader().getGeneratedShader(),
		cx.getSamplerTextures(),
		cx.getRenderState()
	);

	return true;
}

	}
}
