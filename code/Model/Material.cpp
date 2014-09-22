#include "Model/Material.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.Material", Material, PropertyGroup)
		
Material::Material()
:	m_name(L"")
,	m_color(255, 255, 255, 255)
,	m_diffuseTerm(1.0f)
,	m_specularTerm(1.0f)
,	m_specularRoughness(0.8f)
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
,	m_color(255, 255, 255, 255)
,	m_diffuseTerm(1.0f)
,	m_specularTerm(1.0f)
,	m_specularRoughness(0.8f)
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

void Material::setColor(const Color4ub& color)
{
	m_color = color;
}

const Color4ub& Material::getColor() const
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

void Material::setSpecularRoughness(float specularRoughness)
{
	m_specularRoughness = specularRoughness;
}

float Material::getSpecularRoughness() const
{
	return m_specularRoughness;
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

	}
}
