/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/Editor/Hlsl/Hlsl.h"
#include "Render/Dx11/Editor/Hlsl/HlslProgram.h"
#include "Render/Dx11/Editor/Hlsl/HlslContext.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"

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
		computeOutputs.size() >= 1
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
		for (auto computeOutput : computeOutputs)
		{
			if (!cx.emit(computeOutput))
			{
				log::error << L"Unable to generate HLSL shader; emitter failed with compute graph." << Endl;
				log::error << L"\t" << cx.getError() << Endl;
				return false;
			}	
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
