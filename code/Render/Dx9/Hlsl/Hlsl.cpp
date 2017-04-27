/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Render/Dx9/Hlsl/Hlsl.h"
#include "Render/Dx9/Hlsl/HlslContext.h"
#include "Render/Dx9/Hlsl/HlslProgram.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"

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
		log::error << L"Unable to generate HLSL shader; incorrect number of outputs" << Endl;
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
		cx.getVertexShader().getGeneratedShader(cx.needVPos()),
		cx.getPixelShader().getGeneratedShader(cx.needVPos()),
		cx.getVertexShader().getTextures(),
		cx.getVertexShader().getSamplers(),
		cx.getPixelShader().getTextures(),
		cx.getPixelShader().getSamplers(),
		cx.getState()
	);

	return true;
}

	}
}
