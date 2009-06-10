#include "Model/Material.h"

namespace traktor
{
	namespace model
	{
		
T_IMPLEMENT_RTTI_CLASS(L"traktor.model.Material", Material, Object)

Material::Material()
:	m_name(L"")
,	m_diffuseMap(L"")
,	m_diffuseBlendOperator(BoDecal)
,	m_specularMap(L"")
,	m_normalMap(L"")
,	m_color(255, 255, 255, 255)
,	m_diffuseTerm(1.0f)
,	m_specularTerm(1.0f)
,	m_doubleSided(false)
{
}

Material::Material(const std::wstring& name)
:	m_name(name)
,	m_diffuseMap(L"")
,	m_diffuseBlendOperator(BoDecal)
,	m_specularMap(L"")
,	m_normalMap(L"")
,	m_color(255, 255, 255, 255)
,	m_diffuseTerm(1.0f)
,	m_specularTerm(1.0f)
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

void Material::setDiffuseMap(const std::wstring& diffuseMap, BlendOperator diffuseBlendOperator)
{
	m_diffuseMap = diffuseMap;
	m_diffuseBlendOperator = diffuseBlendOperator;
}

const std::wstring& Material::getDiffuseMap() const
{
	return m_diffuseMap;
}

Material::BlendOperator Material::getDiffuseBlendOperator() const
{
	return m_diffuseBlendOperator;
}

void Material::setSpecularMap(const std::wstring& specularMap)
{
	m_specularMap = specularMap;
}

const std::wstring& Material::getSpecularMap() const
{
	return m_specularMap;
}

void Material::setNormalMap(const std::wstring& normalMap)
{
	m_normalMap = normalMap;
}

const std::wstring& Material::getNormalMap() const
{
	return m_normalMap;
}

void Material::setColor(const Color& color)
{
	m_color = color;
}

const Color& Material::getColor() const
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
