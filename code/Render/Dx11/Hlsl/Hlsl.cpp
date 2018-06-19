/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/Hlsl/Hlsl.h"
#include "Render/Dx11/Hlsl/HlslProgram.h"
#include "Render/Dx11/Hlsl/HlslContext.h"
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
	RefArray< ComputeOutput > computeOutputs;

	shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);
	shaderGraph->findNodesOf< ComputeOutput >(computeOutputs);

	if (!(
		(vertexOutputs.size() == 1 && pixelOutputs.size() == 1) ||
		computeOutputs.size() == 1
	))
	{
		log::error << L"Unable to generate HLSL shader; incorrect number of outputs (VS " << vertexOutputs.size() << L", PS " << pixelOutputs.size() << L", CS " << computeOutputs.size() << L")." << Endl;
		return false;
	}

	HlslContext cx(shaderGraph);
	if (computeOutputs.empty())
	{
		if (!cx.emit(pixelOutputs[0]))
		{
			log::error << L"Unable to generate HLSL shader; emitter failed with pixel graph." << Endl;
			log::error << L"\t" << cx.getError() << Endl;
			return false;
		}
		if (!cx.emit(vertexOutputs[0]))
		{
			log::error << L"Unable to generate HLSL shader; emitter failed with vertex graph." << Endl;
			log::error << L"\t" << cx.getError() << Endl;
			return false;
		}
	}
	else
	{
		if (!cx.emit(computeOutputs[0]))
		{
			log::error << L"Unable to generate HLSL shader; emitter failed with compute graph." << Endl;
			log::error << L"\t" << cx.getError() << Endl;
			return false;
		}
	}

	outProgram = HlslProgram(
		cx.getVertexShader().getGeneratedShader(),
		cx.getPixelShader().getGeneratedShader(),
		cx.getComputeShader().getGeneratedShader(),
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
