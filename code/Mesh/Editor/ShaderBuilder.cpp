#include "Mesh/Editor/ShaderBuilder.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Model/Material.h"
#include "Render/ShaderGraph.h"
#include "Render/Nodes.h"
#include "Render/FragmentLinker.h"
#include "Core/Misc/StringUtils.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

template < typename NodeType >
NodeType* findTaggedNode(RefArray< NodeType >& nodes, const std::wstring& tag)
{
	for (typename RefArray< NodeType >::iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		if ((*i)->getComment() == tag)
			return *i;
	}
	return 0;
}

const Guid c_templateOutput(L"{86C22424-6D5B-4747-8E22-A1F9C42604D0}");
const Guid c_templateDiffuse_Const(L"{A2932A34-C041-9E47-ADFD-E262FF535245}");
//const Guid c_templateDiffuse_Texture(L"{F5DEB776-C3A5-B343-B44A-509CBA3BD02F}");
//const Guid c_templateDiffuse_Const_Add_Texture(L"{28A84A8B-4228-B040-87FE-8AA4B652BAE7}");
//const Guid c_templateDiffuse_Const_Sub_Texture(L"{8504FADA-635B-2F4B-A114-2A0DC139637B}");
//const Guid c_templateDiffuse_Const_Mul_Texture(L"{27861F52-3FA1-E745-AA81-88A0DA7953B4}");
//const Guid c_templateDiffuse_Const_Lerp_Texture(L"{5643FB62-74BE-F44B-AA13-CBBDFA5420E3}");
const Guid c_templateSpecular_Const(L"{D39233EA-C270-354E-A961-BEA996968D11}");
//const Guid c_templateSpecular_Texture(L"{A0A22C83-4262-754C-897B-E6C654E41EED}");
const Guid c_templateNormal_Const(L"{4844386D-4066-7D4D-8B14-0E2C3DB2C9CF}");
//const Guid c_templateNormal_Texture(L"{C4E1833D-6C6A-084B-9408-745AF8EDCC03}");

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.ShaderBuilder", ShaderBuilder, Object)

ShaderBuilder::ShaderBuilder(
	db::Database* db,
	db::Group* shaderGroup
)
:	m_db(db)
,	m_shaderGroup(shaderGroup)
{
}

Guid ShaderBuilder::build(const model::Material& material)
{
	render::FragmentLinker linker;

	// Read template shader.
	Ref< render::ShaderGraph > shaderGraph = m_db->getObjectReadOnly< render::ShaderGraph >(c_templateOutput);
	T_ASSERT (shaderGraph);

	// Diffuse fragment.
	//if (!material.getDiffuseMap().empty())
	//{
	//	const Guid* templateDiffuse = 0;
	//	switch (material.getDiffuseBlendOperator())
	//	{
	//	case model::Material::BoDecal:
	//		templateDiffuse = &c_templateDiffuse_Texture;
	//		break;
	//	case model::Material::BoAdd:
	//		templateDiffuse = &c_templateDiffuse_Const_Add_Texture;
	//		break;
	//	case model::Material::BoMultiply:
	//		templateDiffuse = &c_templateDiffuse_Const_Mul_Texture;
	//		break;
	//	case model::Material::BoAlpha:
	//		templateDiffuse = &c_templateDiffuse_Const_Lerp_Texture;
	//		break;
	//	default:
	//		T_BREAKPOINT;
	//	}

	//	Ref< render::ShaderGraph > textureDiffuse = m_db->getObjectReadOnly< render::ShaderGraph >(*templateDiffuse);
	//	T_ASSERT (textureDiffuse);

	//	shaderGraph = linker.merge(textureDiffuse, shaderGraph);
	//	T_ASSERT (shaderGraph);
	//}
	//else
	{
		Ref< render::ShaderGraph > templateDiffuse = m_db->getObjectReadOnly< render::ShaderGraph >(c_templateDiffuse_Const);
		T_ASSERT (templateDiffuse);

		shaderGraph = linker.merge(templateDiffuse, shaderGraph);
		T_ASSERT (shaderGraph);
	}

	// Specular fragment.
	//if (!material.getSpecularMap().empty())
	//{
	//	Ref< render::ShaderGraph > textureSpecular = m_db->getObjectReadOnly< render::ShaderGraph >(c_templateSpecular_Texture);
	//	T_ASSERT (textureSpecular);

	//	shaderGraph = linker.merge(textureSpecular, shaderGraph);
	//	T_ASSERT (shaderGraph);
	//}
	//else
	{
		Ref< render::ShaderGraph > constSpecular = m_db->getObjectReadOnly< render::ShaderGraph >(c_templateSpecular_Const);
		T_ASSERT (constSpecular);

		shaderGraph = linker.merge(constSpecular, shaderGraph);
		T_ASSERT (shaderGraph);
	}

	// Normal fragment.
	//if (!material.getNormalMap().empty())
	//{
	//	Ref< render::ShaderGraph > textureNormal = m_db->getObjectReadOnly< render::ShaderGraph >(c_templateNormal_Texture);
	//	T_ASSERT (textureNormal);

	//	shaderGraph = linker.merge(textureNormal, shaderGraph);
	//	T_ASSERT (shaderGraph);
	//}
	//else
	{
		Ref< render::ShaderGraph > constNormal = m_db->getObjectReadOnly< render::ShaderGraph >(c_templateNormal_Const);
		T_ASSERT (constNormal);

		shaderGraph = linker.merge(constNormal, shaderGraph);
		T_ASSERT (shaderGraph);
	}

	// Change constants.

	// Get references to constants.
	RefArray< render::Color > colorNodes;
	shaderGraph->findNodesOf< render::Color >(colorNodes);
	RefArray< render::Scalar > scalarNodes;
	shaderGraph->findNodesOf< render::Scalar >(scalarNodes);
	//RefArray< render::Sampler > samplerNodes;
	//shaderGraph->findNodesOf< render::Sampler >(samplerNodes);

	// Set constant values to material properties.
	Ref< render::Color > colorNode = findTaggedNode(colorNodes, L"Color");
	if (colorNode)
		colorNode->setColor(material.getColor());

	Ref< render::Scalar > diffuseTermNode = findTaggedNode(scalarNodes, L"Diffuse term");
	if (diffuseTermNode)
		diffuseTermNode->set(material.getDiffuseTerm());

	Ref< render::Scalar > specularRoughnessNode = findTaggedNode(scalarNodes, L"Specular roughness");
	if (specularRoughnessNode)
		specularRoughnessNode->set(material.getSpecularRoughness());

	//// Import and bind texture to samplers.
	//if (!material.getDiffuseMap().empty())
	//{
	//	Ref< render::Sampler > diffuseSamplerNode = findTaggedNode(samplerNodes, L"Diffuse texture");
	//	if (diffuseSamplerNode)
	//	{
	//		std::map< std::wstring, Guid >::const_iterator i = m_textureGuids.find(material.getDiffuseMap());
	//		if (i != m_textureGuids.end())
	//			diffuseSamplerNode->setExternal(i->second);
	//		else
	//			log::error << L"Unable to find diffuse texture \"" << material.getDiffuseMap() << L"\"" << Endl;
	//	}
	//	else
	//		log::warning << L"No \"Diffuse texture\" sampler found in template" << Endl;
	//}

	//if (!material.getSpecularMap().empty())
	//{
	//	Ref< render::Sampler > specularSamplerNode = findTaggedNode(samplerNodes, L"Specular texture");
	//	if (specularSamplerNode)
	//	{
	//		std::map< std::wstring, Guid >::const_iterator i = m_textureGuids.find(material.getSpecularMap());
	//		if (i != m_textureGuids.end())
	//			specularSamplerNode->setExternal(i->second);
	//		else
	//			log::error << L"Unable to find specular texture \"" << material.getSpecularMap() << L"\"" << Endl;
	//	}
	//	else
	//		log::warning << L"No \"Specular texture\" sampler found in template" << Endl;
	//}

	//if (!material.getNormalMap().empty())
	//{
	//	Ref< render::Sampler > normalSamplerNode = findTaggedNode(samplerNodes, L"Normal texture");
	//	if (normalSamplerNode)
	//	{
	//		std::map< std::wstring, Guid >::const_iterator i = m_textureGuids.find(material.getNormalMap());
	//		if (i != m_textureGuids.end())
	//			normalSamplerNode->setExternal(i->second);
	//		else
	//			log::error << L"Unable to find normal texture \"" << material.getNormalMap() << L"\"" << Endl;
	//	}
	//	else
	//		log::warning << L"No \"Normal texture\" sampler found in template" << Endl;
	//}

	// Create new shader instance.
	Ref< db::Instance > instance = m_shaderGroup->createInstance(
		material.getName(),
		shaderGraph,
		db::CifReplaceExisting
	);
	if (!instance || !instance->commit())
	{
		log::error << L"Unable to create shader instance \"" << material.getName() << L"\"" << Endl;
		return Guid();
	}

	return instance->getGuid();
}

	}
}
