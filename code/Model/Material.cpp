/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.model.Material", 0, Material, PropertyGroup)

Material::Material(const std::wstring& name)
:	m_name(name)
{
}

Material::Material(const std::wstring& name, const Color4f& color)
:	m_name(name)
,	m_color(color)
{
}

void Material::setName(const std::wstring& name)
{
	m_name = name;
}

void Material::setDiffuseMap(const Map& diffuseMap)
{
	m_diffuseMap = diffuseMap;
}

void Material::setTransparencyMap(const Map& transparencyMap)
{
	m_transparencyMap = transparencyMap;
}

void Material::setSpecularMap(const Map& specularMap)
{
	m_specularMap = specularMap;
}

void Material::setRoughnessMap(const Map& roughnessMap)
{
	m_roughnessMap = roughnessMap;
}

void Material::setMetalnessMap(const Map& metalnessMap)
{
	m_metalnessMap = metalnessMap;
}

void Material::setReflectiveMap(const Map& reflectiveMap)
{
	m_reflectiveMap = reflectiveMap;
}

void Material::setEmissiveMap(const Map& emissiveMap)
{
	m_emissiveMap = emissiveMap;
}

void Material::setNormalMap(const Map& normalMap)
{
	m_normalMap = normalMap;
}

void Material::setLightMap(const Map& lightMap)
{
	m_lightMap = lightMap;
}

void Material::setColor(const Color4f& color)
{
	m_color = color;
}

void Material::setDiffuseTerm(float diffuseTerm)
{
	m_diffuseTerm = diffuseTerm;
}

void Material::setSpecularTerm(float specularTerm)
{
	m_specularTerm = specularTerm;
}

void Material::setRoughness(float roughness)
{
	m_roughness = roughness;
}

void Material::setMetalness(float metalness)
{
	m_metalness = metalness;
}

void Material::setTransparency(float transparency)
{
	m_transparency = transparency;
}

void Material::setEmissive(float emissive)
{
	m_emissive = emissive;
}

void Material::setReflective(float reflective)
{
	m_reflective = reflective;
}

void Material::setBlendOperator(BlendOperator blendOperator)
{
	m_blendOperator = blendOperator;
}

void Material::setDoubleSided(bool doubleSided)
{
	m_doubleSided = doubleSided;
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
	s >> MemberEnum< BlendOperator >(L"blendOperator", m_blendOperator, c_BlendOperatorKeys);
	s >> Member< bool >(L"doubleSided", m_doubleSided);
}

void Material::Map::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< std::wstring >(L"channel", channel);
	s >> Member< bool >(L"anisotropic", anisotropic);
	s >> Member< Guid >(L"texture", texture);
}

	}
}
