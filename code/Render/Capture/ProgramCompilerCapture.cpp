/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Capture/ProgramCompilerCapture.h"
#include "Render/Capture/ProgramResourceCapture.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramCompilerCapture", ProgramCompilerCapture, IProgramCompiler)

ProgramCompilerCapture::ProgramCompilerCapture(IProgramCompiler* compiler)
:	m_compiler(compiler)
{
}

const wchar_t* ProgramCompilerCapture::getPlatformSignature() const
{
	return m_compiler->getPlatformSignature();
}

Ref< ProgramResource > ProgramCompilerCapture::compile(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	int32_t optimize,
	bool validate,
	Stats* outStats
) const
{
	Ref< ProgramResource > resource = m_compiler->compile(shaderGraph, settings, optimize, validate, outStats);
	if (!resource)
		return 0;

	Ref< ProgramResourceCapture > resourceCapture = new ProgramResourceCapture();
	resourceCapture->m_embedded = resource;

	// Record all uniforms used in shader.
	shaderGraph->findNodesOf< Uniform >(resourceCapture->m_uniforms);
	shaderGraph->findNodesOf< IndexedUniform >(resourceCapture->m_indexedUniforms);

	// Keep copy of readable shader in capture.
	m_compiler->generate(shaderGraph, settings, optimize, resourceCapture->m_vertexShader, resourceCapture->m_pixelShader, resourceCapture->m_computeShader);

	return resourceCapture;
}

bool ProgramCompilerCapture::generate(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	int32_t optimize,
	std::wstring& outVertexShader,
	std::wstring& outPixelShader,
	std::wstring& outComputeShader
) const
{
	return m_compiler->generate(shaderGraph, settings, optimize, outVertexShader, outPixelShader, outComputeShader);
}

	}
}
