/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Editor/VertexShaderGenerator.h"

#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Editor/IPipelineDepends.h"
#include "Model/Model.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphValidator.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/FragmentLinker.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"

namespace traktor::mesh
{
namespace
{

const Guid c_meshShaderTemplate(L"{E657266C-4925-1A40-9225-0776ACC3B0E8}");
const Guid c_meshVertexInterface(L"{4015ACBD-D998-6243-B379-21BB383B864E}");

// Fragments from world MaterialShaderGenerator.
const Guid c_materialInterface(L"{139CACBD-2A79-5644-B9BC-B113F66D50EA}");
const Guid c_tplLightMapParams(L"{2449B257-5B2A-5242-86F9-32105E1F1771}");
const Guid c_implLightMapNull(L"{F8EAEDCD-67C6-B540-A9D0-40141A7FA267}");
const Guid c_implLightMap(L"{DD1F6C98-F5E2-D34B-A5FB-B21CCE3034A2}");
const Guid c_implTexCoordSelect0(L"{D235FD2F-5ED9-5B49-B3F0-14F03B6D8748}");
const Guid c_implTexCoordSelect1(L"{0269F15C-2543-6D4A-ADC0-4DC584976AAF}");

class FragmentReaderAdapter : public render::FragmentLinker::IFragmentReader
{
public:
	FragmentReaderAdapter(const std::function< Ref< const render::ShaderGraph >(const Guid& fragmentId) >& resolve)
		: m_resolve(resolve)
	{
	}

	virtual Ref< const render::ShaderGraph > read(const Guid& fragmentGuid) const
	{
		return m_resolve(fragmentGuid);
	}

private:
	std::function< Ref< const render::ShaderGraph >(const Guid& fragmentId) > m_resolve;
};

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.VertexShaderGenerator", VertexShaderGenerator, Object)

VertexShaderGenerator::VertexShaderGenerator(const std::function< Ref< const render::ShaderGraph >(const Guid& fragmentId) >& resolve)
	: m_resolve(resolve)
{
}

Ref< render::ShaderGraph > VertexShaderGenerator::generateMesh(
	const model::Model& model,
	const model::Material& material,
	const render::ShaderGraph* meshSurfaceShaderGraph,
	const Guid& vertexShaderGuid) const
{
	// Create a mutable material mesh shader.
	Ref< render::ShaderGraph > meshShaderGraph = DeepClone(m_resolve(c_meshShaderTemplate)).create< render::ShaderGraph >();
	if (!meshShaderGraph)
		return nullptr;

	const uint32_t lightMapChannel = model.getTexCoordChannel(L"Lightmap");

	// Insert surface shader into mesh shader.
	{
		RefArray< render::External > resolveNodes;
		for (auto externalNode : meshShaderGraph->findNodesOf< render::External >())
		{
			const Guid& fragmentGuid = externalNode->getFragmentGuid();
			T_ASSERT(fragmentGuid.isValid());

			if (fragmentGuid == c_materialInterface)
				resolveNodes.push_back(externalNode);
			else if (fragmentGuid == c_tplLightMapParams)
			{
				if (lightMapChannel == model::c_InvalidIndex)
					externalNode->setFragmentGuid(c_implLightMapNull);
				else
					externalNode->setFragmentGuid(c_implLightMap);
				resolveNodes.push_back(externalNode);
			}
		}
		T_FATAL_ASSERT(!resolveNodes.empty());

		// Determine texture channel fragments.
		for (auto node : meshShaderGraph->getNodes())
		{
			const std::wstring comment = trim(node->getComment());
			if (comment == L"Tag_LightmapTexCoord")
			{
				render::External* externalNode = mandatory_non_null_type_cast< render::External* >(node);
				externalNode->setFragmentGuid(lightMapChannel == 0 ? c_implTexCoordSelect0 : c_implTexCoordSelect1);
				resolveNodes.push_back(externalNode);
			}
		}

		// Resolve mesh shader; load all patched fragments and merge into a complete shader.
		meshShaderGraph = render::FragmentLinker([&](const Guid& fragmentGuid) -> Ref< const render::ShaderGraph > {
			if (fragmentGuid == c_materialInterface)
				return DeepClone(meshSurfaceShaderGraph).create< render::ShaderGraph >();
			else
				return m_resolve(fragmentGuid);
		}).resolve(meshShaderGraph, resolveNodes, false);
		if (!meshShaderGraph)
			return nullptr;
	}

	// Replace vertex interface with concrete implementation fragment.
	bool replacedVertexInterface = false;
	for (auto externalNode : meshShaderGraph->findNodesOf< render::External >())
	{
		const Guid& fragmentGuid = externalNode->getFragmentGuid();
		if (fragmentGuid == c_meshVertexInterface)
		{
			externalNode->setFragmentGuid(vertexShaderGuid);
			replacedVertexInterface = true;
		}
	}
	if (!replacedVertexInterface)
		return nullptr;

	return meshShaderGraph;
}

void VertexShaderGenerator::addDependencies(editor::IPipelineDepends* pipelineDepends)
{
	pipelineDepends->addDependency(c_meshShaderTemplate, editor::PdfUse);
	pipelineDepends->addDependency(c_meshVertexInterface, editor::PdfUse);
}

}
