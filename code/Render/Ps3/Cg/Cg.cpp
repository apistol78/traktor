/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Render/Ps3/Cg/Cg.h"
#include "Render/Ps3/Cg/CgProgram.h"
#include "Render/Ps3/Cg/CgContext.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Shader/Nodes.h"

namespace traktor
{
	namespace render
	{

bool Cg::generate(
	const ShaderGraph* shaderGraph,
	CgProgram& outProgram
)
{
	RefArray< VertexOutput > vertexOutputs;
	RefArray< PixelOutput > pixelOutputs;

	shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);

	if (vertexOutputs.size() != 1 || pixelOutputs.size() != 1)
	{
		log::error << L"Unable to generate CG shader; incorrect number of outputs" << Endl;
		return false;
	}

	CgContext cx(shaderGraph);

	if (!cx.getEmitter().emit(cx, pixelOutputs[0]))
	{
		log::error << L"Unable to generate CG shader; emitter failed with pixel graph" << Endl;
		return false;
	}

	if (!cx.getEmitter().emit(cx, vertexOutputs[0]))
	{
		log::error << L"Unable to generate CG shader; emitter failed with vertex graph" << Endl;
		return false;
	}

	outProgram = CgProgram(
		cx.getVertexShader().getGeneratedShader(cx.needVPos()),
		cx.getPixelShader().getGeneratedShader(cx.needVPos()),
		cx.getVertexShader().getSamplerTextures(),
		cx.getPixelShader().getSamplerTextures(),
		cx.getRenderState(),
		cx.getRegisterCount()
	);

	return true;
}

	}
}
