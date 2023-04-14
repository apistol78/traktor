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
#include "Render/Editor/Shader/ShaderGraphValidator.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/FragmentLinker.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

const Guid c_materialShader(L"{CCDB27F2-644B-0742-857D-680E846B5BA3}");

// Templates
const Guid c_tplDiffuseParams(L"{4AC7418D-FF43-FE40-ADDC-33A162636FDC}");
const Guid c_tplEmissiveParams(L"{1E35F0A7-23A9-EA49-A518-125A77BAD564}");
const Guid c_tplNormalParams(L"{77489017-FBE8-4A4F-B11A-FDE48C69E021}");
const Guid c_tplOutput(L"{6DA4BE0A-BE19-4440-9B08-FC3FD1FFECDC}");
const Guid c_tplTransparencyParams(L"{052265E6-233C-754C-A297-9369803ADB88}");
const Guid c_tplLightMapParams(L"{2449B257-5B2A-5242-86F9-32105E1F1771}");
const Guid c_tplRoughnessParams(L"{709A171A-6050-1249-AAA7-5AAE428B956C}");
const Guid c_tplSpecularParams(L"{78C32C63-EE8D-AB4C-B2CF-E3FF242980DA}");
const Guid c_tplMetalnessParams(L"{43E7FEE9-043B-A242-B031-BC274995A3A8}");
const Guid c_tplVertexParams(L"{AEBE83FB-68D4-9D45-A672-0A8487A197CD}");
const Guid c_tplTexCoordSelect(L"{60640268-E70B-0540-A073-49DC82E734A2}");

// Implementations
const Guid c_implDiffuseConst(L"{BA68E2CA-77EB-684E-AD2B-0CD4BC35608D}");
const Guid c_implDiffuseVertex(L"{A3DC951A-8BAC-BF40-AFEC-6C47DAF2313F}");
const Guid c_implDiffuseMap(L"{EE7D62D6-B5A8-DC48-8328-A3513B998DD4}");
const Guid c_implEmissiveConst(L"{61A41113-D9F9-964A-9D90-B7A686058A26}");
const Guid c_implEmissiveMap(L"{AA813CBA-5007-2F49-9254-153646162932}");
const Guid c_implNormalConst(L"{5D881AE1-B99D-8941-B949-4E95AEF1CB7A}");
const Guid c_implNormalMap(L"{8CA655BD-E17B-5A48-B6C6-3FDBC1D4F97D}");
const Guid c_implOutputAdd(L"{321B8969-32D7-D44A-BF91-B056E4728DE2}");
const Guid c_implOutputAlpha(L"{1CDA749C-D713-974F-8E84-895AFEE8D552}");
const Guid c_implOutputAlphaTest(L"{902D6E55-9B18-774A-874B-76F55CAC2E4C}");
const Guid c_implOutputDecal(L"{31FD2B2B-3D3C-024F-9AA6-544B73D6009C}");
const Guid c_implOutputMultiply(L"{C635E09A-8DFD-BF40-A863-81301D2388AC}");
const Guid c_implOutputLightMapDecal(L"{4FFCDA64-4B42-DA46-973E-63C740B06A16}");
const Guid c_implOutputLightMapAlphaTest(L"{97914603-BA44-7942-B049-7AE3C3280421}");
const Guid c_implTransparencyConst(L"{FD6737C4-582B-0C41-B1C8-9D4E91B93DD2}");
const Guid c_implTransparencyMap(L"{F7F9394F-912A-9243-A38D-0A1527920FEF}");
const Guid c_implLightMapNull(L"{F8EAEDCD-67C6-B540-A9D0-40141A7FA267}");
const Guid c_implLightMap(L"{DD1F6C98-F5E2-D34B-A5FB-B21CCE3034A2}");
const Guid c_implLightMapDirectional(L"{13B36DED-A778-B846-A2F8-A45E574E4A8C}");
const Guid c_implRoughnessConst(L"{361EE108-403F-C740-B0DF-8B0EAF3155EE}");
const Guid c_implRoughnessMap(L"{2D117E15-90B9-6C4C-B28C-DA18B2AF7B4F}");
const Guid c_implSpecularConst(L"{93DA7E24-5B2F-C24B-8589-FA3D4F025B51}");
const Guid c_implSpecularMap(L"{2E084B99-F346-9E41-84A4-6FEDE3C065FF}");
const Guid c_implMetalnessConst(L"{1760350E-1C62-6B42-B6AA-0D06146A1375}");
const Guid c_implMetalnessMap(L"{FDC79CBC-D1EF-2844-9C17-47EE92A06713}");
const Guid c_implVertex(L"{5CCADFD7-6421-9848-912E-205358848F37}");
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

Ref< render::ShaderGraph > MaterialShaderGenerator::generate(
	const std::function< Ref< const render::ShaderGraph >(const Guid& fragmentId) >& resolve,
	const model::Model& model,
	const model::Material& material,
	const Guid& materialTemplate,
	bool vertexColor
) const
{
	// Read material template shader.
	Guid templateGuid = materialTemplate;
	if (templateGuid.isNull() || !templateGuid.isValid())
		templateGuid = c_materialShader;

	Ref< render::ShaderGraph > materialShaderGraph = DeepClone(resolve(templateGuid)).create< render::ShaderGraph >();
	if (!materialShaderGraph)
		return nullptr;

	// Patch material template shader with concrete implementations of value fetching fragments.
	const Guid& diffuseTexture = material.getDiffuseMap().texture;
	const Guid& roughnessTexture = material.getRoughnessMap().texture;
	const Guid& specularTexture = material.getSpecularMap().texture;
	const Guid& metalnessTexture = material.getMetalnessMap().texture;
	const Guid& transparencyTexture = material.getTransparencyMap().texture;
	const Guid& emissiveTexture = material.getEmissiveMap().texture;
	const Guid& normalTexture = material.getNormalMap().texture;
	const Guid& lightMapTexture = material.getLightMap().texture;

	Guid lightMapDirectionalTexture;
	const std::wstring tmp = material.getProperty< std::wstring >(L"LightMapDirectionalId");
	if (!tmp.empty())
	{
		if (normalTexture.isNotNull())
			lightMapDirectionalTexture = Guid(tmp);
		else
			log::warning << L"Material \"" << material.getName() << L"\" has directional lightmap ID but no normal map associated." << Endl;
	}

	RefArray< render::External > resolveNodes;
	for (auto externalNode : materialShaderGraph->findNodesOf< render::External >())
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
		else if (fragmentGuid == c_tplOutput)
		{
			if (lightMapTexture.isNull())
			{
				switch (material.getBlendOperator())
				{
				default:
				case model::Material::BoDecal:
					externalNode->setFragmentGuid(c_implOutputDecal);
					break;

				case model::Material::BoAdd:
					externalNode->setFragmentGuid(c_implOutputAdd);
					break;

				case model::Material::BoMultiply:
					externalNode->setFragmentGuid(c_implOutputMultiply);
					break;

				case model::Material::BoAlpha:
					externalNode->setFragmentGuid(c_implOutputAlpha);
					break;

				case model::Material::BoAlphaTest:
					externalNode->setFragmentGuid(c_implOutputAlphaTest);
					break;
				}
			}
			else
			{
				if (material.getBlendOperator() != model::Material::BoAlphaTest)
					externalNode->setFragmentGuid(c_implOutputLightMapDecal);
				else
					externalNode->setFragmentGuid(c_implOutputLightMapAlphaTest);
			}
			resolveNodes.push_back(externalNode);
		}
		else if (fragmentGuid == c_tplTransparencyParams)
		{
			if (transparencyTexture.isNull())
				externalNode->setFragmentGuid(c_implTransparencyConst);
			else
				externalNode->setFragmentGuid(c_implTransparencyMap);
			resolveNodes.push_back(externalNode);
		}
		else if (fragmentGuid == c_tplLightMapParams)
		{
			if (lightMapTexture.isNull())
				externalNode->setFragmentGuid(c_implLightMapNull);
			else
			{
				if (lightMapDirectionalTexture.isNotNull())
					externalNode->setFragmentGuid(c_implLightMapDirectional);
				else
					externalNode->setFragmentGuid(c_implLightMap);
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
		else if (fragmentGuid == c_tplVertexParams)
		{
			externalNode->setFragmentGuid(c_implVertex);
			resolveNodes.push_back(externalNode);
		}
	}

	// Determine texture channel fragments.
	for (auto node : materialShaderGraph->getNodes())
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
		else if (comment == L"Tag_LightmapTexCoord")
		{
			render::External* externalNode = mandatory_non_null_type_cast< render::External* >(node);
			const uint32_t channel = model.getTexCoordChannel(material.getLightMap().channel);
			externalNode->setFragmentGuid(channel == 0 ? c_implTexCoordSelect0 : c_implTexCoordSelect1);
			resolveNodes.push_back(externalNode);
		}
	}

	// Resolve material shader; load all patched fragments and merge into a complete shader.
	FragmentReaderAdapter fragmentReader(resolve);
	materialShaderGraph = render::FragmentLinker(fragmentReader).resolve(materialShaderGraph, resolveNodes, false);
	if (!materialShaderGraph)
		return nullptr;

	// Patch constant values, such as colors, from materials into shader.
	for (auto node : materialShaderGraph->getNodes())
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
			propagateAnisotropic(materialShaderGraph, diffuseTextureNode, material.getDiffuseMap().anisotropic);
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
			propagateAnisotropic(materialShaderGraph, emissiveTextureNode, material.getEmissiveMap().anisotropic);
		}
		else if (comment == L"Tag_NormalMap")
		{
			render::Texture* normalTextureNode = mandatory_non_null_type_cast< render::Texture* >(node);
			normalTextureNode->setComment(L"");
			normalTextureNode->setExternal(normalTexture);
			propagateAnisotropic(materialShaderGraph, normalTextureNode, material.getNormalMap().anisotropic);
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
			propagateAnisotropic(materialShaderGraph, roughnessTextureNode, material.getRoughnessMap().anisotropic);
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
			propagateAnisotropic(materialShaderGraph, specularTextureNode, material.getSpecularMap().anisotropic);
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
			propagateAnisotropic(materialShaderGraph, metalnessTextureNode, material.getRoughnessMap().anisotropic);
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
			propagateAnisotropic(materialShaderGraph, transparencyTextureNode, material.getTransparencyMap().anisotropic);
		}
		else if (comment == L"Tag_LightMapDiffuse")
		{
			T_FATAL_ASSERT(lightMapTexture.isNotNull());
			render::Texture* lightMapTextureNode = mandatory_non_null_type_cast< render::Texture* >(node);
			lightMapTextureNode->setComment(L"");
			lightMapTextureNode->setExternal(lightMapTexture);
		}
		else if (comment == L"Tag_LightMapDirectional")
		{
			T_FATAL_ASSERT(lightMapDirectionalTexture.isNotNull());
			render::Texture* lightMapDirectionalTextureNode = mandatory_non_null_type_cast< render::Texture* >(node);
			lightMapDirectionalTextureNode->setComment(L"");
			lightMapDirectionalTextureNode->setExternal(lightMapDirectionalTexture);
		}
	}

#if defined(_DEBUG)
	// Validate integrity and then return complete mesh material shader.
	if (!render::ShaderGraphValidator(materialShaderGraph, templateGuid).validateIntegrity())
		return nullptr;
#endif

	return materialShaderGraph;
}

void MaterialShaderGenerator::addDependencies(editor::IPipelineDepends* pipelineDepends)
{
	pipelineDepends->addDependency(c_materialShader, editor::PdfUse);

	// Templates
	pipelineDepends->addDependency(c_tplDiffuseParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplEmissiveParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplNormalParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplOutput, editor::PdfUse);
	pipelineDepends->addDependency(c_tplTransparencyParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplLightMapParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplRoughnessParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplSpecularParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplMetalnessParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplVertexParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplTexCoordSelect, editor::PdfUse);

	// Implementations
	pipelineDepends->addDependency(c_implDiffuseConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implDiffuseVertex, editor::PdfUse);
	pipelineDepends->addDependency(c_implDiffuseMap, editor::PdfUse);
	pipelineDepends->addDependency(c_implEmissiveConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implEmissiveMap, editor::PdfUse);
	pipelineDepends->addDependency(c_implNormalConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implNormalMap, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputAdd, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputAlpha, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputAlphaTest, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputDecal, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputMultiply, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputLightMapDecal, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputLightMapAlphaTest, editor::PdfUse);
	pipelineDepends->addDependency(c_implTransparencyConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implTransparencyMap, editor::PdfUse);
	pipelineDepends->addDependency(c_implLightMapNull, editor::PdfUse);
	pipelineDepends->addDependency(c_implLightMap, editor::PdfUse);
	pipelineDepends->addDependency(c_implLightMapDirectional, editor::PdfUse);
	pipelineDepends->addDependency(c_implRoughnessConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implRoughnessMap, editor::PdfUse);
	pipelineDepends->addDependency(c_implSpecularConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implSpecularMap, editor::PdfUse);
	pipelineDepends->addDependency(c_implMetalnessConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implMetalnessMap, editor::PdfUse);
	pipelineDepends->addDependency(c_implVertex, editor::PdfUse);
	pipelineDepends->addDependency(c_implTexCoordSelect0, editor::PdfUse);
	pipelineDepends->addDependency(c_implTexCoordSelect1, editor::PdfUse);
}

	}
}
