#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Editor/IPipelineDepends.h"
#include "Mesh/Editor/MaterialShaderGenerator.h"
#include "Model/Material.h"
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
const Guid c_tplDiffuseParams(L"{4AC7418D-FF43-FE40-ADDC-33A162636FDC}");
const Guid c_tplEmissiveParams(L"{1E35F0A7-23A9-EA49-A518-125A77BAD564}");
const Guid c_tplNormalParams(L"{77489017-FBE8-4A4F-B11A-FDE48C69E021}");
const Guid c_tplOutput(L"{6DA4BE0A-BE19-4440-9B08-FC3FD1FFECDC}");
const Guid c_tplTransparencyParams(L"{052265E6-233C-754C-A297-9369803ADB88}");
const Guid c_tplLightMapParams(L"{2449B257-5B2A-5242-86F9-32105E1F1771}");
const Guid c_tplRoughnessParams(L"{709A171A-6050-1249-AAA7-5AAE428B956C}");
const Guid c_tplMetalnessParams(L"{43E7FEE9-043B-A242-B031-BC274995A3A8}");
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
const Guid c_implOutputAdd(L"{321B8969-32D7-D44A-BF91-B056E4728DE2}");
const Guid c_implOutputAlpha(L"{1CDA749C-D713-974F-8E84-895AFEE8D552}");
const Guid c_implOutputDecal(L"{31FD2B2B-3D3C-024F-9AA6-544B73D6009C}");
const Guid c_implOutputMultiply(L"{C635E09A-8DFD-BF40-A863-81301D2388AC}");
const Guid c_implOutputLightMapDecal(L"{4FFCDA64-4B42-DA46-973E-63C740B06A16}");
const Guid c_implTransparencyConst(L"{FD6737C4-582B-0C41-B1C8-9D4E91B93DD2}");
const Guid c_implTransparencyMap0(L"{F7F9394F-912A-9243-A38D-0A1527920FEF}");
const Guid c_implTransparencyMap1(L"{63451E3C-CC4E-A245-B721-8498E0AE5D0D}");
const Guid c_implLightMapNull(L"{F8EAEDCD-67C6-B540-A9D0-40141A7FA267}");
const Guid c_implLightMap0(L"{DD1F6C98-F5E2-D34B-A5FB-B21CCE3034A2}");
const Guid c_implLightMap1(L"{54546782-D141-7C48-BF31-FDAC1161516C}");
const Guid c_implRoughnessConst(L"{361EE108-403F-C740-B0DF-8B0EAF3155EE}");
const Guid c_implRoughnessMap0(L"{2D117E15-90B9-6C4C-B28C-DA18B2AF7B4F}");
const Guid c_implRoughnessMap1(L"{CC075F37-B198-6340-B9C6-E654EE6D3165}");
const Guid c_implMetalnessConst(L"{1760350E-1C62-6B42-B6AA-0D06146A1375}");
const Guid c_implMetalnessMap0(L"{FDC79CBC-D1EF-2844-9C17-47EE92A06713}");
const Guid c_implMetalnessMap1(L"{13D0B4B7-C095-6E4B-B628-94F2D5B0B553}");
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
	Guid roughnessTexture = lookupTexture(textures, material.getRoughnessMap().name);
	Guid metalnessTexture = lookupTexture(textures, material.getMetalnessMap().name);
	Guid transparencyTexture = lookupTexture(textures, material.getTransparencyMap().name);
	Guid emissiveTexture = lookupTexture(textures, material.getEmissiveMap().name);
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
		else if (fragmentGuid == c_tplRoughnessParams)
		{
			if (roughnessTexture.isNull())
				(*i)->setFragmentGuid(c_implRoughnessConst);
			else
				(*i)->setFragmentGuid(material.getRoughnessMap().channel == 0 ? c_implRoughnessMap0 : c_implRoughnessMap1);
		}
		else if (fragmentGuid == c_tplMetalnessParams)
		{
			if (metalnessTexture.isNull())
				(*i)->setFragmentGuid(c_implMetalnessConst);
			else
				(*i)->setFragmentGuid(material.getMetalnessMap().channel == 0 ? c_implMetalnessMap0 : c_implMetalnessMap1);
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
		else if (comment == L"Tag_Roughness")
		{
			render::Scalar* roughnessNode = checked_type_cast< render::Scalar* >(*i);
			roughnessNode->setComment(L"");
			roughnessNode->set(material.getRoughness());
		}
		else if (comment == L"Tag_RoughnessMap")
		{
			render::Texture* roughnessTextureNode = checked_type_cast< render::Texture* >(*i);
			roughnessTextureNode->setComment(L"");
			roughnessTextureNode->setExternal(roughnessTexture);
			propagateAnisotropic(materialShaderGraph, roughnessTextureNode, material.getRoughnessMap().anisotropic);
		}
		else if (comment == L"Tag_Metalness")
		{
			render::Scalar* metalnessNode = checked_type_cast< render::Scalar* >(*i);
			metalnessNode->setComment(L"");
			metalnessNode->set(material.getMetalness());
		}
		else if (comment == L"Tag_MetalnessMap")
		{
			render::Texture* metalnessTextureNode = checked_type_cast< render::Texture* >(*i);
			metalnessTextureNode->setComment(L"");
			metalnessTextureNode->setExternal(metalnessTexture);
			propagateAnisotropic(materialShaderGraph, metalnessTextureNode, material.getRoughnessMap().anisotropic);
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
	pipelineDepends->addDependency(c_tplTransparencyParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplLightMapParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplRoughnessParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplMetalnessParams, editor::PdfUse);
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
	pipelineDepends->addDependency(c_implOutputAdd, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputAlpha, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputDecal, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputMultiply, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputLightMapDecal, editor::PdfUse);
	pipelineDepends->addDependency(c_implTransparencyConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implTransparencyMap0, editor::PdfUse);
	pipelineDepends->addDependency(c_implTransparencyMap1, editor::PdfUse);
	pipelineDepends->addDependency(c_implLightMapNull, editor::PdfUse);
	pipelineDepends->addDependency(c_implLightMap0, editor::PdfUse);
	pipelineDepends->addDependency(c_implLightMap1, editor::PdfUse);
	pipelineDepends->addDependency(c_implRoughnessConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implRoughnessMap0, editor::PdfUse);
	pipelineDepends->addDependency(c_implRoughnessMap1, editor::PdfUse);
	pipelineDepends->addDependency(c_implMetalnessConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implMetalnessMap0, editor::PdfUse);
	pipelineDepends->addDependency(c_implMetalnessMap1, editor::PdfUse);
	pipelineDepends->addDependency(c_implVertex, editor::PdfUse);
}

	}
}
