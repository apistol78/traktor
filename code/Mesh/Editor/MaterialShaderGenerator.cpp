/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Editor/IPipelineDepends.h"
#include "Mesh/Editor/MaterialShaderGenerator.h"
#include "Model/Material.h"
#include "Render/Editor/Shader/ShaderGraphValidator.h"
#include "Render/Resource/FragmentLinker.h"
#include "Render/Shader/External.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

const Guid c_materialShader(L"{CCDB27F2-644B-0742-857D-680E846B5BA3}");
const Guid c_tplDiffuseParams(L"{4AC7418D-FF43-FE40-ADDC-33A162636FDC}");
const Guid c_tplEmissiveParams(L"{1E35F0A7-23A9-EA49-A518-125A77BAD564}");
const Guid c_tplNormalParams(L"{77489017-FBE8-4A4F-B11A-FDE48C69E021}");
const Guid c_tplOutput(L"{6DA4BE0A-BE19-4440-9B08-FC3FD1FFECDC}");
const Guid c_tplRimParams(L"{57310F3A-FEB0-7644-B641-EC3876773470}");
const Guid c_tplReflectionParams(L"{859D3A98-EB16-0F4B-A766-34B6D65096AA}");
const Guid c_tplSpecularParams(L"{68DA66E7-1D9E-FD4C-9692-D947BEA3EBAD}");
const Guid c_tplTransparencyParams(L"{052265E6-233C-754C-A297-9369803ADB88}");
const Guid c_tplLightMapParams(L"{2449B257-5B2A-5242-86F9-32105E1F1771}");
const Guid c_tplVertexParams(L"{AEBE83FB-68D4-9D45-A672-0A8487A197CD}");
const Guid c_implDiffuseConst(L"{BA68E2CA-77EB-684E-AD2B-0CD4BC35608D}");
const Guid c_implDiffuseVertex(L"{A3DC951A-8BAC-BF40-AFEC-6C47DAF2313F}");
const Guid c_implDiffuseMap0(L"{EE7D62D6-B5A8-DC48-8328-A3513B998DD4}");
const Guid c_implDiffuseMap1(L"{85794EB5-58ED-E843-AF20-A21558AC406C}");
const Guid c_implEmissiveConst(L"{61A41113-D9F9-964A-9D90-B7A686058A26}");
const Guid c_implEmissiveMap0(L"{AA813CBA-5007-2F49-9254-153646162932}");
const Guid c_implEmissiveMap1(L"{A08C4322-7F81-1849-9228-34EA1DE93019}");
const Guid c_implNormalConst(L"{5D881AE1-B99D-8941-B949-4E95AEF1CB7A}");
const Guid c_implNormalMap0(L"{8CA655BD-E17B-5A48-B6C6-3FDBC1D4F97D}");
const Guid c_implNormalMap1(L"{C9B1BA07-716A-0349-8A34-BDEAAB818714}");
const Guid c_implRimConst(L"{449F16EF-5C14-4940-A5E1-E1ABF73CC5D7}");
const Guid c_implReflectiveConst(L"{2E6EC61A-E1C2-D545-A29A-B4CBED8914E5}");
const Guid c_implReflectiveMap0(L"{D8F82B95-DC7F-1A4A-9A1C-E0B6CBDEDE4D}");
const Guid c_implReflectiveMap1(L"{E45465CE-F699-5346-BA2E-9B95DE80035C}");
const Guid c_implOutputAdd(L"{321B8969-32D7-D44A-BF91-B056E4728DE2}");
const Guid c_implOutputAlpha(L"{1CDA749C-D713-974F-8E84-895AFEE8D552}");
const Guid c_implOutputDecal(L"{31FD2B2B-3D3C-024F-9AA6-544B73D6009C}");
const Guid c_implOutputMultiply(L"{C635E09A-8DFD-BF40-A863-81301D2388AC}");
const Guid c_implOutputLightMapDecal(L"{4FFCDA64-4B42-DA46-973E-63C740B06A16}");
const Guid c_implSpecularConst(L"{6D818781-04A2-2E48-9340-BEFB493F1F9E}");
const Guid c_implSpecularMap0(L"{208A6FD9-8591-294F-BEAB-B2B872992960}");
const Guid c_implSpecularMap1(L"{11FB0173-2120-704D-A310-B63172A20768}");
const Guid c_implTransparencyConst(L"{FD6737C4-582B-0C41-B1C8-9D4E91B93DD2}");
const Guid c_implTransparencyMap0(L"{F7F9394F-912A-9243-A38D-0A1527920FEF}");
const Guid c_implTransparencyMap1(L"{63451E3C-CC4E-A245-B721-8498E0AE5D0D}");
const Guid c_implLightMapNull(L"{F8EAEDCD-67C6-B540-A9D0-40141A7FA267}");
const Guid c_implLightMap0(L"{DD1F6C98-F5E2-D34B-A5FB-B21CCE3034A2}");
const Guid c_implLightMap1(L"{54546782-D141-7C48-BF31-FDAC1161516C}");
const Guid c_implVertex(L"{5CCADFD7-6421-9848-912E-205358848F37}");

class FragmentReaderAdapter : public render::FragmentLinker::IFragmentReader
{
public:
	FragmentReaderAdapter(db::Database* database)
	:	m_database(database)
	{
	}

	virtual Ref< const render::ShaderGraph > read(const Guid& fragmentGuid) const
	{
		return m_database->getObjectReadOnly< render::ShaderGraph >(fragmentGuid);
	}

private:
	Ref< db::Database > m_database;
};

Guid lookupTexture(const std::map< std::wstring, Guid >& textures, const std::wstring& name)
{
	std::map< std::wstring, Guid >::const_iterator i = textures.find(name);
	return i != textures.end() ? i->second : Guid();
}

void propagateAnisotropic(render::ShaderGraph* shaderGraph, render::Texture* textureNode, bool anisotropic)
{
	std::vector< const render::InputPin* > destinationPins;
	shaderGraph->findDestinationPins(textureNode->getOutputPin(0), destinationPins);
	for (std::vector< const render::InputPin* >::iterator i = destinationPins.begin(); i != destinationPins.end(); ++i)
	{
		render::Sampler* samplerNode = dynamic_type_cast< render::Sampler* >((*i)->getNode());
		if (samplerNode)
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
	db::Database* database,
	const model::Material& material,
	const Guid& materialTemplate,
	const std::map< std::wstring, Guid >& textures,
	bool vertexColor
) const
{
	// Read material template shader.
	Guid templateGuid = materialTemplate;
	if (templateGuid.isNull() || !templateGuid.isValid())
		templateGuid = c_materialShader;

	Ref< render::ShaderGraph > materialShaderGraph = database->getObjectReadOnly< render::ShaderGraph >(templateGuid);
	if (!materialShaderGraph)
		return 0;

	// Patch material template shader with concrete implementations of value fetching fragments.
	Guid diffuseTexture = lookupTexture(textures, material.getDiffuseMap().name);
	Guid specularTexture = lookupTexture(textures, material.getSpecularMap().name);
	Guid transparencyTexture = lookupTexture(textures, material.getTransparencyMap().name);
	Guid emissiveTexture = lookupTexture(textures, material.getEmissiveMap().name);
	Guid reflectiveTexture = lookupTexture(textures, material.getReflectiveMap().name);
	Guid normalTexture = lookupTexture(textures, material.getNormalMap().name);
	Guid lightMapTexture = lookupTexture(textures, material.getLightMap().name);

	RefArray< render::External > externalNodes;
	materialShaderGraph->findNodesOf< render::External >(externalNodes);

	for (RefArray< render::External >::iterator i = externalNodes.begin(); i != externalNodes.end(); ++i)
	{
		const Guid& fragmentGuid = (*i)->getFragmentGuid();
		T_ASSERT (fragmentGuid.isValid());

		if (fragmentGuid == c_tplDiffuseParams)
		{
			if (diffuseTexture.isNull())
			{
				if (!vertexColor)
					(*i)->setFragmentGuid(c_implDiffuseConst);
				else
					(*i)->setFragmentGuid(c_implDiffuseVertex);
			}
			else
				(*i)->setFragmentGuid(material.getDiffuseMap().channel == 0 ? c_implDiffuseMap0 : c_implDiffuseMap1);
		}
		else if (fragmentGuid == c_tplEmissiveParams)
		{
			if (emissiveTexture.isNull())
				(*i)->setFragmentGuid(c_implEmissiveConst);
			else
				(*i)->setFragmentGuid(material.getEmissiveMap().channel == 0 ? c_implEmissiveMap0 : c_implEmissiveMap1);
		}
		else if (fragmentGuid == c_tplNormalParams)
		{
			if (normalTexture.isNull())
				(*i)->setFragmentGuid(c_implNormalConst);
			else
				(*i)->setFragmentGuid(material.getNormalMap().channel == 0 ? c_implNormalMap0 : c_implNormalMap1);
		}
		else if (fragmentGuid == c_tplOutput)
		{
			if (lightMapTexture.isNull())
			{
				switch (material.getBlendOperator())
				{
				default:
				case model::Material::BoDecal:
					(*i)->setFragmentGuid(c_implOutputDecal);
					break;
				case model::Material::BoAdd:
					(*i)->setFragmentGuid(c_implOutputAdd);
					break;
				case model::Material::BoMultiply:
					(*i)->setFragmentGuid(c_implOutputMultiply);
					break;
				case model::Material::BoAlpha:
					(*i)->setFragmentGuid(c_implOutputAlpha);
					break;
				}
			}
			else
				(*i)->setFragmentGuid(c_implOutputLightMapDecal);
		}
		else if (fragmentGuid == c_tplRimParams)
			(*i)->setFragmentGuid(c_implRimConst);
		else if (fragmentGuid == c_tplReflectionParams)
		{
			if (reflectiveTexture.isNull())
				(*i)->setFragmentGuid(c_implReflectiveConst);
			else
				(*i)->setFragmentGuid(material.getReflectiveMap().channel == 0 ? c_implReflectiveMap0 : c_implReflectiveMap1);
		}
		else if (fragmentGuid == c_tplSpecularParams)
		{
			if (specularTexture.isNull())
				(*i)->setFragmentGuid(c_implSpecularConst);
			else
				(*i)->setFragmentGuid(material.getSpecularMap().channel == 0 ? c_implSpecularMap0 : c_implSpecularMap1);
		}
		else if (fragmentGuid == c_tplTransparencyParams)
		{
			if (transparencyTexture.isNull())
				(*i)->setFragmentGuid(c_implTransparencyConst);
			else
				(*i)->setFragmentGuid(material.getTransparencyMap().channel == 0 ? c_implTransparencyMap0 : c_implTransparencyMap1);
		}
		else if (fragmentGuid == c_tplLightMapParams)
		{
			if (lightMapTexture.isNull())
				(*i)->setFragmentGuid(c_implLightMapNull);
			else
				(*i)->setFragmentGuid(material.getLightMap().channel == 0 ? c_implLightMap0 : c_implLightMap1);
		}
		else if (fragmentGuid == c_tplVertexParams)
			(*i)->setFragmentGuid(c_implVertex);
	}

	// Resolve material shader; load all patched fragments and merge into a complete shader.
	FragmentReaderAdapter fragmentReader(database);
	materialShaderGraph = render::FragmentLinker(fragmentReader).resolve(materialShaderGraph, false);
	if (!materialShaderGraph)
		return 0;

	// Patch constant values, such as colors, from materials into shader.
	const RefArray< render::Node >& nodes = materialShaderGraph->getNodes();
	for (RefArray< render::Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		std::wstring comment = (*i)->getComment();
		if (comment == L"Tag_DiffuseColor")
		{
			render::Color* colorNode = checked_type_cast< render::Color* >(*i);
			colorNode->setComment(L"");
			colorNode->setColor(material.getColor());
		}
		else if (comment == L"Tag_DiffuseTerm")
		{
			render::Scalar* diffuseTermNode = checked_type_cast< render::Scalar* >(*i);
			diffuseTermNode->setComment(L"");
			diffuseTermNode->set(material.getDiffuseTerm());
		}
		else if (comment == L"Tag_DiffuseMap")
		{
			render::Texture* diffuseTextureNode = checked_type_cast< render::Texture* >(*i);
			diffuseTextureNode->setComment(L"");
			diffuseTextureNode->setExternal(diffuseTexture);
			propagateAnisotropic(materialShaderGraph, diffuseTextureNode, material.getDiffuseMap().anisotropic);
		}
		else if (comment == L"Tag_Emissive")
		{
			render::Scalar* emissiveNode = checked_type_cast< render::Scalar* >(*i);
			emissiveNode->setComment(L"");
			emissiveNode->set(material.getEmissive());
		}
		else if (comment == L"Tag_EmissiveMap")
		{
			render::Texture* emissiveTextureNode = checked_type_cast< render::Texture* >(*i);
			emissiveTextureNode->setComment(L"");
			emissiveTextureNode->setExternal(emissiveTexture);
			propagateAnisotropic(materialShaderGraph, emissiveTextureNode, material.getEmissiveMap().anisotropic);
		}
		else if (comment == L"Tag_NormalMap")
		{
			render::Texture* normalTextureNode = checked_type_cast< render::Texture* >(*i);
			normalTextureNode->setComment(L"");
			normalTextureNode->setExternal(normalTexture);
			propagateAnisotropic(materialShaderGraph, normalTextureNode, material.getNormalMap().anisotropic);
		}
		else if (comment == L"Tag_Reflective")
		{
			render::Scalar* reflectiveNode = checked_type_cast< render::Scalar* >(*i);
			reflectiveNode->setComment(L"");
			reflectiveNode->set(material.getReflective());
		}
		else if (comment == L"Tag_ReflectiveMap")
		{
			render::Texture* reflectiveTextureNode = checked_type_cast< render::Texture* >(*i);
			reflectiveTextureNode->setComment(L"");
			reflectiveTextureNode->setExternal(reflectiveTexture);
			propagateAnisotropic(materialShaderGraph, reflectiveTextureNode, material.getReflectiveMap().anisotropic);
		}
		else if (comment == L"Tag_RimIntensity")
		{
			render::Scalar* rimIntensityNode = checked_type_cast< render::Scalar* >(*i);
			rimIntensityNode->setComment(L"");
			rimIntensityNode->set(material.getRimLightIntensity());
		}
		else if (comment == L"Tag_SpecularTerm")
		{
			render::Scalar* specularTermNode = checked_type_cast< render::Scalar* >(*i);
			specularTermNode->setComment(L"");
			specularTermNode->set(material.getSpecularTerm());
		}
		else if (comment == L"Tag_SpecularMap")
		{
			render::Texture* specularTextureNode = checked_type_cast< render::Texture* >(*i);
			specularTextureNode->setComment(L"");
			specularTextureNode->setExternal(specularTexture);
			propagateAnisotropic(materialShaderGraph, specularTextureNode, material.getSpecularMap().anisotropic);
		}
		else if (comment == L"Tag_SpecularRoughness")
		{
			render::Scalar* specularRoughnessNode = checked_type_cast< render::Scalar* >(*i);
			specularRoughnessNode->setComment(L"");
			specularRoughnessNode->set(material.getSpecularRoughness());
		}
		else if (comment == L"Tag_Metalness")
		{
			render::Scalar* metalnessNode = checked_type_cast< render::Scalar* >(*i);
			metalnessNode->setComment(L"");
			metalnessNode->set(material.getMetalness());
		}
		else if (comment == L"Tag_Transparency")
		{
			render::Scalar* transparencyNode = checked_type_cast< render::Scalar* >(*i);
			transparencyNode->setComment(L"");
			transparencyNode->set(material.getTransparency());
		}
		else if (comment == L"Tag_TransparencyMap")
		{
			render::Texture* transparencyTextureNode = checked_type_cast< render::Texture* >(*i);
			transparencyTextureNode->setComment(L"");
			transparencyTextureNode->setExternal(transparencyTexture);
			propagateAnisotropic(materialShaderGraph, transparencyTextureNode, material.getTransparencyMap().anisotropic);
		}
		else if (comment == L"Tag_LightMap")
		{
			render::Texture* lightMapTextureNode = checked_type_cast< render::Texture* >(*i);
			lightMapTextureNode->setComment(L"");
			lightMapTextureNode->setExternal(lightMapTexture);
		}
		else if (comment == L"Tag_LightMapRange")
		{
			render::Scalar* lightMapRangeNode = checked_type_cast< render::Scalar* >(*i);
			lightMapRangeNode->setComment(L"");
			lightMapRangeNode->set(material.getLightMapRange());
		}
	}

	// Validate integrity and then return complete mesh material shader.
	render::ShaderGraphValidator(materialShaderGraph).validateIntegrity();
	return materialShaderGraph;
}

void MaterialShaderGenerator::addDependencies(editor::IPipelineDepends* pipelineDepends)
{
	pipelineDepends->addDependency(c_materialShader, editor::PdfUse);
	pipelineDepends->addDependency(c_tplDiffuseParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplEmissiveParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplNormalParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplOutput, editor::PdfUse);
	pipelineDepends->addDependency(c_tplRimParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplReflectionParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplSpecularParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplTransparencyParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplLightMapParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplVertexParams, editor::PdfUse);
	pipelineDepends->addDependency(c_implDiffuseConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implDiffuseVertex, editor::PdfUse);
	pipelineDepends->addDependency(c_implDiffuseMap0, editor::PdfUse);
	pipelineDepends->addDependency(c_implDiffuseMap1, editor::PdfUse);
	pipelineDepends->addDependency(c_implEmissiveConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implEmissiveMap0, editor::PdfUse);
	pipelineDepends->addDependency(c_implEmissiveMap1, editor::PdfUse);
	pipelineDepends->addDependency(c_implNormalConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implNormalMap0, editor::PdfUse);
	pipelineDepends->addDependency(c_implNormalMap1, editor::PdfUse);
	pipelineDepends->addDependency(c_implRimConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implReflectiveConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implReflectiveMap0, editor::PdfUse);
	pipelineDepends->addDependency(c_implReflectiveMap1, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputAdd, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputAlpha, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputDecal, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputMultiply, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputLightMapDecal, editor::PdfUse);
	pipelineDepends->addDependency(c_implSpecularConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implSpecularMap0, editor::PdfUse);
	pipelineDepends->addDependency(c_implSpecularMap1, editor::PdfUse);
	pipelineDepends->addDependency(c_implTransparencyConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implTransparencyMap0, editor::PdfUse);
	pipelineDepends->addDependency(c_implTransparencyMap1, editor::PdfUse);
	pipelineDepends->addDependency(c_implLightMapNull, editor::PdfUse);
	pipelineDepends->addDependency(c_implLightMap0, editor::PdfUse);
	pipelineDepends->addDependency(c_implLightMap1, editor::PdfUse);
	pipelineDepends->addDependency(c_implVertex, editor::PdfUse);
}

	}
}
