/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vrfy/Editor/ProgramCompilerVrfy.h"

#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Vrfy/ProgramResourceVrfy.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerVrfy", 0, ProgramCompilerVrfy, IProgramCompiler)

bool ProgramCompilerVrfy::create(IProgramCompiler* embedded)
{
	m_compiler = embedded;
	return true;
}

const wchar_t* ProgramCompilerVrfy::getRendererSignature() const
{
	return m_compiler->getRendererSignature();
}

Ref< ProgramResource > ProgramCompilerVrfy::compile(
	const ShaderGraph* shaderGraph,
	const ShaderModule* shaderModule,
	const PropertyGroup* settings,
	const std::wstring& name,
	std::list< Error >& outErrors) const
{
	// Compile program using wrapped compiler.
	Ref< ProgramResource > resource = m_compiler->compile(shaderGraph, shaderModule, settings, name, outErrors);
	if (!resource)
		return nullptr;

	// Embed into custom resource, append debug data to program useful for capturing.
	Ref< ProgramResourceVrfy > resourceVrfy = new ProgramResourceVrfy();
	resourceVrfy->m_requireRayTracing = resource->requireRayTracing();
	resourceVrfy->m_embedded = resource;

	// Record all uniforms used in shader.
	for (auto uniform : shaderGraph->findNodesOf< Uniform >())
		resourceVrfy->m_uniforms.push_back({ uniform->getParameterName(),
			uniform->getParameterType(),
			0 });
	for (auto indexedUniform : shaderGraph->findNodesOf< IndexedUniform >())
		resourceVrfy->m_uniforms.push_back({ indexedUniform->getParameterName(),
			indexedUniform->getParameterType(),
			indexedUniform->getLength() });

	// Keep copy of readable shader in capture.
	Output output;
	m_compiler->generate(
		shaderGraph,
		shaderModule,
		settings,
		name,
		output);
	resourceVrfy->m_vertexShader = output.vertex;
	resourceVrfy->m_pixelShader = output.pixel;
	resourceVrfy->m_computeShader = output.compute;

	return resourceVrfy;
}

bool ProgramCompilerVrfy::generate(
	const ShaderGraph* shaderGraph,
	const ShaderModule* shaderModule,
	const PropertyGroup* settings,
	const std::wstring& name,
	Output& output) const
{
	// Just let the wrapped compiler generate source.
	return m_compiler->generate(shaderGraph, shaderModule, settings, name, output);
}

}
