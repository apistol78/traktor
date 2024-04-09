/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Editor/IPipelineDepends.h"
#include "Mesh/Editor/MaterialShaderGenerator.h"
#include "Model/Model.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/FragmentLinker.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphValidator.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

const Guid c_meshSurfaceTemplate(L"{CCDB27F2-644B-0742-857D-680E846B5BA3}");
const Guid c_meshShaderTemplate(L"{E657266C-4925-1A40-9225-0776ACC3B0E8}");

const Guid c_meshVertexInterface(L"{4015ACBD-D998-6243-B379-21BB383B864E}");
const Guid c_meshSurfaceInterface(L"{139CACBD-2A79-5644-B9BC-B113F66D50EA}");

// Templates
const Guid c_tplDiffuseParams(L"{4AC7418D-FF43-FE40-ADDC-33A162636FDC}");
const Guid c_tplEmissiveParams(L"{1E35F0A7-23A9-EA49-A518-125A77BAD564}");
const Guid c_tplNormalParams(L"{77489017-FBE8-4A4F-B11A-FDE48C69E021}");
const Guid c_tplTransparencyParams(L"{052265E6-233C-754C-A297-9369803ADB88}");
const Guid c_tplLightMapParams(L"{2449B257-5B2A-5242-86F9-32105E1F1771}");
const Guid c_tplRoughnessParams(L"{709A171A-6050-1249-AAA7-5AAE428B956C}");
const Guid c_tplSpecularParams(L"{78C32C63-EE8D-AB4C-B2CF-E3FF242980DA}");
const Guid c_tplMetalnessParams(L"{43E7FEE9-043B-A242-B031-BC274995A3A8}");
const Guid c_tplTexCoordSelect(L"{60640268-E70B-0540-A073-49DC82E734A2}");

// Implementations
const Guid c_implDiffuseConst(L"{BA68E2CA-77EB-684E-AD2B-0CD4BC35608D}");
const Guid c_implDiffuseVertex(L"{A3DC951A-8BAC-BF40-AFEC-6C47DAF2313F}");
const Guid c_implDiffuseMap(L"{EE7D62D6-B5A8-DC48-8328-A3513B998DD4}");
const Guid c_implEmissiveConst(L"{61A41113-D9F9-964A-9D90-B7A686058A26}");
const Guid c_implEmissiveMap(L"{AA813CBA-5007-2F49-9254-153646162932}");
const Guid c_implNormalConst(L"{5D881AE1-B99D-8941-B949-4E95AEF1CB7A}");
const Guid c_implNormalMap(L"{8CA655BD-E17B-5A48-B6C6-3FDBC1D4F97D}");
const Guid c_implTransparencyOpaque(L"{F2A2949F-47BB-C147-8DBE-0BB18861CA3A}");
const Guid c_implTransparencyConst(L"{FD6737C4-582B-0C41-B1C8-9D4E91B93DD2}");
const Guid c_implTransparencyMap(L"{F7F9394F-912A-9243-A38D-0A1527920FEF}");
const Guid c_implLightMapNull(L"{F8EAEDCD-67C6-B540-A9D0-40141A7FA267}");
const Guid c_implLightMap(L"{DD1F6C98-F5E2-D34B-A5FB-B21CCE3034A2}");
const Guid c_implRoughnessConst(L"{361EE108-403F-C740-B0DF-8B0EAF3155EE}");
const Guid c_implRoughnessMap(L"{2D117E15-90B9-6C4C-B28C-DA18B2AF7B4F}");
const Guid c_implSpecularConst(L"{93DA7E24-5B2F-C24B-8589-FA3D4F025B51}");
const Guid c_implSpecularMap(L"{2E084B99-F346-9E41-84A4-6FEDE3C065FF}");
const Guid c_implMetalnessConst(L"{1760350E-1C62-6B42-B6AA-0D06146A1375}");
const Guid c_implMetalnessMap(L"{FDC79CBC-D1EF-2844-9C17-47EE92A06713}");
const Guid c_implTexCoordSelect0(L"{D235FD2F-5ED9-5B49-B3F0-14F03B6D8748}");
const Guid c_implTexCoordSelect1(L"{0269F15C-2543-6D4A-ADC0-4DC584976AAF}");

class FragmentReaderAdapter : public render::FragmentLinker::IFragmentReader
{
public:
	FragmentReaderAdapter(const std::function< Ref< const render::ShaderGraph >(const Guid& fragmentId) >& resolve)
	:	m_resolve(resolve)
	{
	}

	virtual Ref< const render::ShaderGraph > read(const Guid& fragmentGuid) const
	{
		return m_resolve(fragmentGuid);
	}

private:
	std::function< Ref< const render::ShaderGraph >(const Guid& fragmentId) > m_resolve;
};

void propagateAnisotropic(render::ShaderGraph* shaderGraph, render::Texture* textureNode, bool anisotropic)
{
	for (auto destinationPin : shaderGraph->findDestinationPins(textureNode->getOutputPin(0)))
	{
		if (auto samplerNode = dynamic_type_cast< render::Sampler* >(destinationPin->getNode()))
		{
			render::SamplerState samplerState = samplerNode->getSamplerState();
			samplerState.useAnisotropic = anisotropic;
			samplerNode->setSamplerState(samplerState);
		}
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MaterialShaderGenerator", MaterialShaderGenerator, Object)

MaterialShaderGenerator::MaterialShaderGenerator(const std::function< Ref< const render::ShaderGraph >(const Guid& fragmentId) >& resolve)
:	m_resolve(resolve)
{
}

Ref< render::ShaderGraph > MaterialShaderGenerator::generateSurface(
	const model::Model& model,
	const model::Material& material,
	bool vertexColor,
	bool decalResponse
) const
{
	// Create a mutable material surface shader.
	Ref< render::ShaderGraph > meshSurfaceShaderGraph = DeepClone(m_resolve(c_meshSurfaceTemplate)).create< render::ShaderGraph >();
	if (!meshSurfaceShaderGraph)
		return nullptr;

	// Patch material template shader with concrete implementations of value fetching fragments.
	const Guid& diffuseTexture = material.getDiffuseMap().texture;
	const Guid& roughnessTexture = material.getRoughnessMap().texture;
	const Guid& specularTexture = material.getSpecularMap().texture;
	const Guid& metalnessTexture = material.getMetalnessMap().texture;
	const Guid& transparencyTexture = material.getTransparencyMap().texture;
	const Guid& emissiveTexture = material.getEmissiveMap().texture;
	const Guid& normalTexture = material.getNormalMap().texture;

	// Replace external fragments with implementations.
	RefArray< render::External > resolveNodes;
	for (auto externalNode : meshSurfaceShaderGraph->findNodesOf< render::External >())
	{
		const Guid& fragmentGuid = externalNode->getFragmentGuid();
		T_ASSERT(fragmentGuid.isValid());

		if (fragmentGuid == c_tplDiffuseParams)
		{
			if (diffuseTexture.isNull())
			{
				if (!vertexColor)
					externalNode->setFragmentGuid(c_implDiffuseConst);
				else
					externalNode->setFragmentGuid(c_implDiffuseVertex);
			}
			else
				externalNode->setFragmentGuid(c_implDiffuseMap);
			resolveNodes.push_back(externalNode);
		}
		else if (fragmentGuid == c_tplEmissiveParams)
		{
			if (emissiveTexture.isNull())
				externalNode->setFragmentGuid(c_implEmissiveConst);
			else
				externalNode->setFragmentGuid(c_implEmissiveMap);
			resolveNodes.push_back(externalNode);
		}
		else if (fragmentGuid == c_tplNormalParams)
		{
			if (normalTexture.isNull())
				externalNode->setFragmentGuid(c_implNormalConst);
			else
				externalNode->setFragmentGuid(c_implNormalMap);
			resolveNodes.push_back(externalNode);
		}
		else if (fragmentGuid == c_tplTransparencyParams)
		{
			if (material.getBlendOperator() == model::Material::BoDecal)
				externalNode->setFragmentGuid(c_implTransparencyOpaque);
			else
			{
				if (transparencyTexture.isNull())
					externalNode->setFragmentGuid(c_implTransparencyConst);
				else
					externalNode->setFragmentGuid(c_implTransparencyMap);
			}
			resolveNodes.push_back(externalNode);
		}
		else if (fragmentGuid == c_tplRoughnessParams)
		{
			if (roughnessTexture.isNull())
				externalNode->setFragmentGuid(c_implRoughnessConst);
			else
				externalNode->setFragmentGuid(c_implRoughnessMap);
			resolveNodes.push_back(externalNode);
		}
		else if (fragmentGuid == c_tplSpecularParams)
		{
			if (specularTexture.isNull())
				externalNode->setFragmentGuid(c_implSpecularConst);
			else
				externalNode->setFragmentGuid(c_implSpecularMap);
			resolveNodes.push_back(externalNode);
		}
		else if (fragmentGuid == c_tplMetalnessParams)
		{
			if (metalnessTexture.isNull())
				externalNode->setFragmentGuid(c_implMetalnessConst);
			else
				externalNode->setFragmentGuid(c_implMetalnessMap);
			resolveNodes.push_back(externalNode);
		}
	}

	// Determine texture channel fragments.
	for (auto node : meshSurfaceShaderGraph->getNodes())
	{
		const std::wstring comment = node->getComment();
		if (comment == L"Tag_DiffuseTexCoord")
		{
			render::External* externalNode = mandatory_non_null_type_cast< render::External* >(node);
			const uint32_t channel = model.getTexCoordChannel(material.getDiffuseMap().channel);
			externalNode->setFragmentGuid(channel == 0 ? c_implTexCoordSelect0 : c_implTexCoordSelect1);
			resolveNodes.push_back(externalNode);
		}
		else if (comment == L"Tag_EmissiveTexCoord")
		{
			render::External* externalNode = mandatory_non_null_type_cast< render::External* >(node);
			const uint32_t channel = model.getTexCoordChannel(material.getEmissiveMap().channel);
			externalNode->setFragmentGuid(channel == 0 ? c_implTexCoordSelect0 : c_implTexCoordSelect1);
			resolveNodes.push_back(externalNode);
		}
		else if (comment == L"Tag_NormalTexCoord")
		{
			render::External* externalNode = mandatory_non_null_type_cast< render::External* >(node);
			const uint32_t channel = model.getTexCoordChannel(material.getNormalMap().channel);
			externalNode->setFragmentGuid(channel == 0 ? c_implTexCoordSelect0 : c_implTexCoordSelect1);
			resolveNodes.push_back(externalNode);
		}
		else if (comment == L"Tag_RoughnessTexCoord")
		{
			render::External* externalNode = mandatory_non_null_type_cast< render::External* >(node);
			const uint32_t channel = model.getTexCoordChannel(material.getRoughnessMap().channel);
			externalNode->setFragmentGuid(channel == 0 ? c_implTexCoordSelect0 : c_implTexCoordSelect1);
			resolveNodes.push_back(externalNode);
		}
		else if (comment == L"Tag_SpecularTexCoord")
		{
			render::External* externalNode = mandatory_non_null_type_cast< render::External* >(node);
			const uint32_t channel = model.getTexCoordChannel(material.getSpecularMap().channel);
			externalNode->setFragmentGuid(channel == 0 ? c_implTexCoordSelect0 : c_implTexCoordSelect1);
			resolveNodes.push_back(externalNode);
		}
		else if (comment == L"Tag_MetalnessTexCoord")
		{
			render::External* externalNode = mandatory_non_null_type_cast< render::External* >(node);
			const uint32_t channel = model.getTexCoordChannel(material.getMetalnessMap().channel);
			externalNode->setFragmentGuid(channel == 0 ? c_implTexCoordSelect0 : c_implTexCoordSelect1);
			resolveNodes.push_back(externalNode);
		}
		else if (comment == L"Tag_TransparencyTexCoord")
		{
			render::External* externalNode = mandatory_non_null_type_cast< render::External* >(node);
			const uint32_t channel = model.getTexCoordChannel(material.getTransparencyMap().channel);
			externalNode->setFragmentGuid(channel == 0 ? c_implTexCoordSelect0 : c_implTexCoordSelect1);
			resolveNodes.push_back(externalNode);
		}
	}

	// Resolve material shader; load all patched fragments and merge into a complete shader.
	FragmentReaderAdapter fragmentReader(m_resolve);
	meshSurfaceShaderGraph = render::FragmentLinker(fragmentReader).resolve(meshSurfaceShaderGraph, resolveNodes, false);
	if (!meshSurfaceShaderGraph)
		return nullptr;

	// Patch constant values, such as colors, from materials into shader.
	for (auto node : meshSurfaceShaderGraph->getNodes())
	{
		const std::wstring comment = node->getComment();
		if (comment == L"Tag_DiffuseColor")
		{
			render::Color* colorNode = mandatory_non_null_type_cast< render::Color* >(node);
			colorNode->setComment(L"");
			colorNode->setColor(material.getColor());
		}
		else if (comment == L"Tag_DiffuseMap")
		{
			render::Texture* diffuseTextureNode = mandatory_non_null_type_cast< render::Texture* >(node);
			diffuseTextureNode->setComment(L"");
			diffuseTextureNode->setExternal(diffuseTexture);
			propagateAnisotropic(meshSurfaceShaderGraph, diffuseTextureNode, material.getDiffuseMap().anisotropic);
		}
		else if (comment == L"Tag_Emissive")
		{
			render::Scalar* emissiveNode = mandatory_non_null_type_cast< render::Scalar* >(node);
			emissiveNode->setComment(L"");
			emissiveNode->set(material.getEmissive());
		}
		else if (comment == L"Tag_EmissiveMap")
		{
			render::Texture* emissiveTextureNode = mandatory_non_null_type_cast< render::Texture* >(node);
			emissiveTextureNode->setComment(L"");
			emissiveTextureNode->setExternal(emissiveTexture);
			propagateAnisotropic(meshSurfaceShaderGraph, emissiveTextureNode, material.getEmissiveMap().anisotropic);
		}
		else if (comment == L"Tag_NormalMap")
		{
			render::Texture* normalTextureNode = mandatory_non_null_type_cast< render::Texture* >(node);
			normalTextureNode->setComment(L"");
			normalTextureNode->setExternal(normalTexture);
			propagateAnisotropic(meshSurfaceShaderGraph, normalTextureNode, material.getNormalMap().anisotropic);
		}
		else if (comment == L"Tag_Roughness")
		{
			render::Scalar* roughnessNode = mandatory_non_null_type_cast< render::Scalar* >(node);
			roughnessNode->setComment(L"");
			roughnessNode->set(material.getRoughness());
		}
		else if (comment == L"Tag_RoughnessMap")
		{
			render::Texture* roughnessTextureNode = mandatory_non_null_type_cast< render::Texture* >(node);
			roughnessTextureNode->setComment(L"");
			roughnessTextureNode->setExternal(roughnessTexture);
			propagateAnisotropic(meshSurfaceShaderGraph, roughnessTextureNode, material.getRoughnessMap().anisotropic);
		}
		else if (comment == L"Tag_Specular")
		{
			render::Scalar* specularNode = mandatory_non_null_type_cast< render::Scalar* >(node);
			specularNode->setComment(L"");
			specularNode->set(material.getSpecularTerm());
		}
		else if (comment == L"Tag_SpecularMap")
		{
			render::Texture* specularTextureNode = mandatory_non_null_type_cast< render::Texture* >(node);
			specularTextureNode->setComment(L"");
			specularTextureNode->setExternal(specularTexture);
			propagateAnisotropic(meshSurfaceShaderGraph, specularTextureNode, material.getSpecularMap().anisotropic);
		}
		else if (comment == L"Tag_Metalness")
		{
			render::Scalar* metalnessNode = mandatory_non_null_type_cast< render::Scalar* >(node);
			metalnessNode->setComment(L"");
			metalnessNode->set(material.getMetalness());
		}
		else if (comment == L"Tag_MetalnessMap")
		{
			render::Texture* metalnessTextureNode = mandatory_non_null_type_cast< render::Texture* >(node);
			metalnessTextureNode->setComment(L"");
			metalnessTextureNode->setExternal(metalnessTexture);
			propagateAnisotropic(meshSurfaceShaderGraph, metalnessTextureNode, material.getRoughnessMap().anisotropic);
		}
		else if (comment == L"Tag_Transparency")
		{
			render::Scalar* transparencyNode = mandatory_non_null_type_cast< render::Scalar* >(node);
			transparencyNode->setComment(L"");
			transparencyNode->set(material.getTransparency());
		}
		else if (comment == L"Tag_TransparencyMap")
		{
			render::Texture* transparencyTextureNode = mandatory_non_null_type_cast< render::Texture* >(node);
			transparencyTextureNode->setComment(L"");
			transparencyTextureNode->setExternal(transparencyTexture);
			propagateAnisotropic(meshSurfaceShaderGraph, transparencyTextureNode, material.getTransparencyMap().anisotropic);
		}
		else if (comment == L"Tag_DecalResponse")
		{
			render::Scalar* decalResponseNode = mandatory_non_null_type_cast< render::Scalar* >(node);
			decalResponseNode->setComment(L"");
			decalResponseNode->set(decalResponse ? 1.0f : 0.0f);			
		}
	}

	// Validate integrity and then return complete mesh material shader.
	if (!render::ShaderGraphValidator(meshSurfaceShaderGraph, c_meshSurfaceTemplate).validateIntegrity())
		return nullptr;

	return meshSurfaceShaderGraph;
}

Ref< render::ShaderGraph >  MaterialShaderGenerator::combineSurface(
	const render::ShaderGraph* customSurfaceShaderGraph,
	const render::ShaderGraph* materialSurfaceShaderGraph
) const
{
	// Resolve custom surface shader; merge in the generated material surface shader.

	// Find all references to mesh surface interface fragment.
	RefArray< render::External > resolveNodes;
	for (auto externalNode : customSurfaceShaderGraph->findNodesOf< render::External >())
	{
		const Guid& fragmentGuid = externalNode->getFragmentGuid();
		if (fragmentGuid == c_meshSurfaceInterface)
			resolveNodes.push_back(externalNode);
	}

	// Link all found fragments.
	Ref< render::ShaderGraph > meshShaderGraph = render::FragmentLinker([&](const Guid& fragmentGuid) -> Ref< const render::ShaderGraph > {
		T_FATAL_ASSERT(fragmentGuid == c_meshSurfaceInterface);
		return DeepClone(materialSurfaceShaderGraph).create< render::ShaderGraph >();
	}).resolve(customSurfaceShaderGraph, resolveNodes, false);

	return meshShaderGraph;
}

Ref< render::ShaderGraph > MaterialShaderGenerator::generateMesh(
	const model::Model& model,
	const model::Material& material,
	const render::ShaderGraph* meshSurfaceShaderGraph,
	const Guid& vertexShaderGuid
) const
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

			if (fragmentGuid == c_meshSurfaceInterface)
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
			const std::wstring comment = node->getComment();
			if (comment == L"Tag_LightmapTexCoord")
			{
				render::External* externalNode = mandatory_non_null_type_cast< render::External* >(node);
				externalNode->setFragmentGuid(lightMapChannel == 0 ? c_implTexCoordSelect0 : c_implTexCoordSelect1);
				resolveNodes.push_back(externalNode);
			}
		}

		// Resolve mesh shader; load all patched fragments and merge into a complete shader.
		meshShaderGraph = render::FragmentLinker([&](const Guid& fragmentGuid) -> Ref< const render::ShaderGraph > {
			if (fragmentGuid == c_meshSurfaceInterface)
				return DeepClone(meshSurfaceShaderGraph).create< render::ShaderGraph >();
			else
				return m_resolve(fragmentGuid);
		}).resolve(meshShaderGraph, resolveNodes, false);
		if (!meshShaderGraph)
			return nullptr;
	}

	// Replace vertex interface with concrete implementation fragment.
	for (auto externalNode : meshShaderGraph->findNodesOf< render::External >())
	{
		const Guid& fragmentGuid = externalNode->getFragmentGuid();
		if (fragmentGuid == c_meshVertexInterface)
			externalNode->setFragmentGuid(vertexShaderGuid);
	}

	return meshShaderGraph;
}

void MaterialShaderGenerator::addDependencies(editor::IPipelineDepends* pipelineDepends)
{
	pipelineDepends->addDependency(c_meshSurfaceTemplate, editor::PdfUse);
	pipelineDepends->addDependency(c_meshShaderTemplate, editor::PdfUse);

	pipelineDepends->addDependency(c_meshVertexInterface, editor::PdfUse);
	pipelineDepends->addDependency(c_meshSurfaceInterface, editor::PdfUse);

	// Templates
	pipelineDepends->addDependency(c_tplDiffuseParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplEmissiveParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplNormalParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplTransparencyParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplLightMapParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplRoughnessParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplSpecularParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplMetalnessParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplTexCoordSelect, editor::PdfUse);

	// Implementations
	pipelineDepends->addDependency(c_implDiffuseConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implDiffuseVertex, editor::PdfUse);
	pipelineDepends->addDependency(c_implDiffuseMap, editor::PdfUse);
	pipelineDepends->addDependency(c_implEmissiveConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implEmissiveMap, editor::PdfUse);
	pipelineDepends->addDependency(c_implNormalConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implNormalMap, editor::PdfUse);
	pipelineDepends->addDependency(c_implTransparencyOpaque, editor::PdfUse);
	pipelineDepends->addDependency(c_implTransparencyConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implTransparencyMap, editor::PdfUse);
	pipelineDepends->addDependency(c_implLightMapNull, editor::PdfUse);
	pipelineDepends->addDependency(c_implLightMap, editor::PdfUse);
	pipelineDepends->addDependency(c_implRoughnessConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implRoughnessMap, editor::PdfUse);
	pipelineDepends->addDependency(c_implSpecularConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implSpecularMap, editor::PdfUse);
	pipelineDepends->addDependency(c_implMetalnessConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implMetalnessMap, editor::PdfUse);
	pipelineDepends->addDependency(c_implTexCoordSelect0, editor::PdfUse);
	pipelineDepends->addDependency(c_implTexCoordSelect1, editor::PdfUse);
}

	}
}
