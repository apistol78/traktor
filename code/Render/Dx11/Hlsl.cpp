#include "Render/Dx11/Platform.h"
#include "Render/Dx11/Hlsl.h"
#include "Render/Dx11/HlslProgram.h"
#include "Render/Dx11/HlslContext.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Shader/Nodes.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

bool Hlsl::generate(
	const ShaderGraph* shaderGraph,
	HlslProgram& outProgram
)
{
	RefArray< VertexOutput > vertexOutputs;
	RefArray< PixelOutput > pixelOutputs;

	shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);

	if (vertexOutputs.size() != 1 || pixelOutputs.size() != 1)
	{
		log::error << L"Unable to generate HLSL shader; incorrect number of outputs (VS " << vertexOutputs.size() << L", PS " << pixelOutputs.size() << L")" << Endl;
		return false;
	}

	HlslContext cx(shaderGraph);

	if (!cx.getEmitter().emit(cx, pixelOutputs[0]))
	{
		log::error << L"Unable to generate HLSL shader; emitter failed with pixel graph" << Endl;
		return false;
	}

	if (!cx.getEmitter().emit(cx, vertexOutputs[0]))
	{
		log::error << L"Unable to generate HLSL shader; emitter failed with vertex graph" << Endl;
		return false;
	}

	outProgram = HlslProgram(
		cx.getVertexShader().getGeneratedShader(),
		cx.getPixelShader().getGeneratedShader(),
		cx.getD3DRasterizerDesc(),
		cx.getD3DDepthStencilDesc(),
		cx.getD3DBlendDesc(),
		cx.getStencilReference(),
		cx.getVertexShader().getSamplers(),
		cx.getPixelShader().getSamplers()
	);

	return true;
}

	}
}
