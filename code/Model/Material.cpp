#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberBitMask.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"
#include "Model/Material.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.Material", 0, Material, PropertyGroup)

Material::Material()
:	m_name(L"")
,	m_color(1.0f, 1.0f, 1.0f, 1.0f)
,	m_diffuseTerm(1.0f)
,	m_specularTerm(1.0f)
,	m_roughness(0.8f)
,	m_metalness(0.0f)
,	m_transparency(0.0f)
,	m_emissive(0.0f)
,	m_reflective(0.0f)
,	m_rimLightIntensity(0.0f)
,	m_blendOperator(BoDecal)
,	m_doubleSided(false)
{
}

Material::Material(const std::wstring& name)
:	m_name(name)
,	m_color(1.0f, 1.0f, 1.0f, 1.0f)
,	m_diffuseTerm(1.0f)
,	m_specularTerm(1.0f)
,	m_roughness(0.8f)
,	m_metalness(0.0f)
,	m_transparency(0.0f)
,	m_emissive(0.0f)
,	m_reflective(0.0f)
,	m_rimLightIntensity(0.0f)
,	m_blendOperator(BoDecal)
,	m_doubleSided(false)
{
}

Material::Material(const std::wstring& name, const Color4f& color)
:	m_name(name)
,	m_color(color)
,	m_diffuseTerm(1.0f)
,	m_specularTerm(1.0f)
,	m_roughness(0.8f)
,	m_metalness(0.0f)
,	m_transparency(0.0f)
,	m_emissive(0.0f)
,	m_reflective(0.0f)
,	m_rimLightIntensity(0.0f)
,	m_blendOperator(BoDecal)
,	m_doubleSided(false)
{
}

void Material::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& Material::getName() const
{
	return m_name;
}

void Material::setDiffuseMap(const Map& diffuseMap)
{
	m_diffuseMap = diffuseMap;
}

const Material::Map& Material::getDiffuseMap() const
{
	return m_diffuseMap;
}

void Material::setTransparencyMap(const Map& transparencyMap)
{
	m_transparencyMap = transparencyMap;
}

const Material::Map& Material::getTransparencyMap() const
{
	return m_transparencyMap;
}

void Material::setSpecularMap(const Map& specularMap)
{
	m_specularMap = specularMap;
}

const Material::Map& Material::getSpecularMap() const
{
	return m_specularMap;
}

void Material::setRoughnessMap(const Map& roughnessMap)
{
	m_roughnessMap = roughnessMap;
}

const Material::Map& Material::getRoughnessMap() const
{
	return m_roughnessMap;
}

void Material::setMetalnessMap(const Map& metalnessMap)
{
	m_metalnessMap = metalnessMap;
}

const Material::Map& Material::getMetalnessMap() const
{
	return m_metalnessMap;
}

void Material::setReflectiveMap(const Map& reflectiveMap)
{
	m_reflectiveMap = reflectiveMap;
}

const Material::Map& Material::getReflectiveMap() const
{
	return m_reflectiveMap;
}

void Material::setEmissiveMap(const Map& emissiveMap)
{
	m_emissiveMap = emissiveMap;
}

const Material::Map& Material::getEmissiveMap() const
{
	return m_emissiveMap;
}

void Material::setNormalMap(const Map& normalMap)
{
	m_normalMap = normalMap;
}

const Material::Map& Material::getNormalMap() const
{
	return m_normalMap;
}

void Material::setLightMap(const Map& lightMap)
{
	m_lightMap = lightMap;
}

const Material::Map& Material::getLightMap() const
{
	return m_lightMap;
}

void Material::setColor(const Color4f& color)
{
	m_color = color;
}

const Color4f& Material::getColor() const
{
	return m_color;
}

void Material::setDiffuseTerm(float diffuseTerm)
{
	m_diffuseTerm = diffuseTerm;
}

float Material::getDiffuseTerm() const
{
	return m_diffuseTerm;
}

void Material::setSpecularTerm(float specularTerm)
{
	m_specularTerm = specularTerm;
}

float Material::getSpecularTerm() const
{
	return m_specularTerm;
}

void Material::setRoughness(float roughness)
{
	m_roughness = roughness;
}

float Material::getRoughness() const
{
	return m_roughness;
}

void Material::setMetalness(float metalness)
{
	m_metalness = metalness;
}

float Material::getMetalness() const
{
	return m_metalness;
}

void Material::setTransparency(float transparency)
{
	m_transparency = transparency;
}

float Material::getTransparency() const
{
	return m_transparency;
}

void Material::setEmissive(float emissive)
{
	m_emissive = emissive;
}

float Material::getEmissive() const
{
	return m_emissive;
}

void Material::setReflective(float reflective)
{
	m_reflective = reflective;
}

float Material::getReflective() const
{
	return m_reflective;
}

void Material::setRimLightIntensity(float rimLightIntensity)
{
	m_rimLightIntensity = rimLightIntensity;
}

float Material::getRimLightIntensity() const
{
	return m_rimLightIntensity;
}

void Material::setBlendOperator(BlendOperator blendOperator)
{
	m_blendOperator = blendOperator;
}

Material::BlendOperator Material::getBlendOperator() const
{
	return m_blendOperator;
}

void Material::setDoubleSided(bool doubleSided)
{
	m_doubleSided = doubleSided;
}

bool Material::isDoubleSided() const
{
	return m_doubleSided;
}

void Material::serialize(ISerializer& s)
{
	const MemberEnum< BlendOperator >::Key c_BlendOperatorKeys[] =
	{
		{ L"BoDecal", BoDecal },
		{ L"BoAdd", BoAdd },
		{ L"BoMultiply", BoMultiply },
		{ L"BoAlpha", BoAlpha },
		{ L"BoAlphaTest", BoAlphaTest },
		{ 0 }
	};

	PropertyGroup::serialize(s);

	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberComposite< Map >(L"diffuseMap", m_diffuseMap);
	s >> MemberComposite< Map >(L"specularMap", m_specularMap);
	s >> MemberComposite< Map >(L"roughnessMap", m_roughnessMap);
	s >> MemberComposite< Map >(L"metalnessMap", m_metalnessMap);
	s >> MemberComposite< Map >(L"transparencyMap", m_transparencyMap);
	s >> MemberComposite< Map >(L"emissiveMap", m_emissiveMap);
	s >> MemberComposite< Map >(L"reflectiveMap", m_reflectiveMap);
	s >> MemberComposite< Map >(L"normalMap", m_normalMap);
	s >> MemberComposite< Map >(L"lightMap", m_lightMap);
	s >> Member< Color4f >(L"color", m_color);
	s >> Member< float >(L"diffuseTerm", m_diffuseTerm);
	s >> Member< float >(L"specularTerm", m_specularTerm);
	s >> Member< float >(L"roughness", m_roughness);
	s >> Member< float >(L"metalness", m_metalness);
	s >> Member< float >(L"transparency", m_transparency);
	s >> Member< float >(L"emissive", m_emissive);
	s >> Member< float >(L"reflective", m_reflective);
	s >> Member< float >(L"rimLightIntensity", m_rimLightIntensity);
	s >> MemberEnum< BlendOperator >(L"blendOperator", m_blendOperator, c_BlendOperatorKeys);
	s >> Member< bool >(L"doubleSided", m_doubleSided);
}

void Material::Map::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< uint32_t >(L"channel", channel);
	s >> Member< bool >(L"anisotropic", anisotropic);
	s >> Member< Guid >(L"texture", texture);
}

	}
}
