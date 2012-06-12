#include "Model/Material.h"

namespace traktor
{
	namespace model
	{
		
Material::Material()
:	m_name(L"")
,	m_diffuseMap(L"")
,	m_specularMap(L"")
,	m_emissiveMap(L"")
,	m_reflectiveMap(L"")
,	m_normalMap(L"")
,	m_color(255, 255, 255, 255)
,	m_diffuseTerm(1.0f)
,	m_specularTerm(1.0f)
,	m_specularRoughness(0.8f)
,	m_emissive(0.0f)
,	m_reflective(0.0f)
,	m_rimLightIntensity(0.0f)
,	m_blendOperator(BoDecal)
,	m_doubleSided(false)
{
}

Material::Material(const std::wstring& name)
:	m_name(name)
,	m_diffuseMap(L"")
,	m_specularMap(L"")
,	m_emissiveMap(L"")
,	m_reflectiveMap(L"")
,	m_normalMap(L"")
,	m_color(255, 255, 255, 255)
,	m_diffuseTerm(1.0f)
,	m_specularTerm(1.0f)
,	m_specularRoughness(0.8f)
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

void Material::setDiffuseMap(const std::wstring& diffuseMap)
{
	m_diffuseMap = diffuseMap;
}

const std::wstring& Material::getDiffuseMap() const
{
	return m_diffuseMap;
}

void Material::setSpecularMap(const std::wstring& specularMap)
{
	m_specularMap = specularMap;
}

const std::wstring& Material::getSpecularMap() const
{
	return m_specularMap;
}

void Material::setEmissiveMap(const std::wstring& emissiveMap)
{
	m_emissiveMap = emissiveMap;
}

const std::wstring& Material::getReflectiveMap() const
{
	return m_reflectiveMap;
}

void Material::setReflectiveMap(const std::wstring& reflectiveMap)
{
	m_reflectiveMap = reflectiveMap;
}

const std::wstring& Material::getEmissiveMap() const
{
	return m_emissiveMap;
}

void Material::setNormalMap(const std::wstring& normalMap)
{
	m_normalMap = normalMap;
}

const std::wstring& Material::getNormalMap() const
{
	return m_normalMap;
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
