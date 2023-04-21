/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vrfy/ProgramResourceVrfy.h"
#include "Render/Vrfy/Editor/ProgramCompilerVrfy.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramCompilerVrfy", ProgramCompilerVrfy, IProgramCompiler)

ProgramCompilerVrfy::ProgramCompilerVrfy(IProgramCompiler* compiler)
:	m_compiler(compiler)
{
}

const wchar_t* ProgramCompilerVrfy::getRendererSignature() const
{
	return m_compiler->getRendererSignature();
}

Ref< ProgramResource > ProgramCompilerVrfy::compile(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	const std::wstring& name,
	std::list< Error >& outErrors
) const
{
	// Compile program using wrapped compiler.
	Ref< ProgramResource > resource = m_compiler->compile(shaderGraph, settings, name, outErrors);
	if (!resource)
		return nullptr;

	// Embed into custom resource, append debug data to program useful for capturing.
	Ref< ProgramResourceVrfy > resourceVrfy = new ProgramResourceVrfy();
	resourceVrfy->m_embedded = resource;

	// Record all uniforms used in shader.
	// shaderGraph->findNodesOf< Uniform >(resourceVrfy->m_uniforms);
	// shaderGraph->findNodesOf< IndexedUniform >(resourceVrfy->m_indexedUniforms);

	// Keep copy of readable shader in capture.
	m_compiler->generate(
		shaderGraph,
		settings,
		name,
		resourceVrfy->m_vertexShader,
		resourceVrfy->m_pixelShader,
		resourceVrfy->m_computeShader
	);

	return resourceVrfy;
}

bool ProgramCompilerVrfy::generate(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	const std::wstring& name,
	std::wstring& outVertexShader,
	std::wstring& outPixelShader,
	std::wstring& outComputeShader
) const
{
	// Just let the wrapped compiler generate source.
	return m_compiler->generate(shaderGraph, settings, name, outVertexShader, outPixelShader, outComputeShader);
}

	}
}
