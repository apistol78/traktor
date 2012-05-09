#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Render/OpenGL/Glsl.h"
#include "Render/OpenGL/GlslProgram.h"
#include "Render/OpenGL/GlslContext.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"

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

	bool requireDerivatives = cx.getRequireDerivatives();
	bool requireTranspose = cx.getRequireTranspose();

	outProgram = GlslProgram(
		cx.getVertexShader().getGeneratedShader(false, requireTranspose),
		cx.getFragmentShader().getGeneratedShader(requireDerivatives, requireTranspose),
		cx.getTextures(),
		cx.getSamplers(),
		cx.getRenderState()
	);

	return true;
}

	}
}
