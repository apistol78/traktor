/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Config.h"
#include "Core/Guid.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Math/Color4f.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

class Image;

}

namespace traktor::model
{

/*! Material descriptor.
 * \ingroup Model
 */
class T_DLLCLASS Material : public PropertyGroup
{
	T_RTTI_CLASS;

public:
	enum BlendOperator
	{
		BoDecal = 0,
		BoAdd = 1,
		BoMultiply = 2,
		BoAlpha = 3,
		BoAlphaTest = 4
	};

	struct Map
	{
		std::wstring name;
		std::wstring channel;
		bool anisotropic = false;
		Guid texture;
		Ref< drawing::Image > image;

		Map() = default;

		Map(const std::wstring& name_, const std::wstring& channel_, bool anisotropic_ = true, const Guid& texture_ = Guid(), drawing::Image* image_ = nullptr)
		:	name(name_)
		,	channel(channel_)
		,	anisotropic(anisotropic_)
		,	texture(texture_)
		,	image(image_)
		{
		}

		void serialize(ISerializer& s);
	};

	Material() = default;

	explicit Material(const std::wstring& name);

	explicit Material(const std::wstring& name, const Color4f& color);

	void setName(const std::wstring& name);

	const std::wstring& getName() const { return m_name; }

	void setDiffuseMap(const Map& diffuseMap);

	Map& getDiffuseMap(){ return m_diffuseMap; }

	const Map& getDiffuseMap() const { return m_diffuseMap; }

	void setSpecularMap(const Map& specularMap);

	Map& getSpecularMap(){ return m_specularMap; }

	const Map& getSpecularMap() const { return m_specularMap; }

	void setRoughnessMap(const Map& roughnessMap);

	Map& getRoughnessMap() { return m_roughnessMap; }

	const Map& getRoughnessMap() const { return m_roughnessMap; }

	void setMetalnessMap(const Map& metalnessMap);

	Map& getMetalnessMap() { return m_metalnessMap; }

	const Map& getMetalnessMap() const { return m_metalnessMap; }

	void setTransparencyMap(const Map& transparencyMap);

	Map& getTransparencyMap() { return m_transparencyMap; }

	const Map& getTransparencyMap() const { return m_transparencyMap; }

	void setEmissiveMap(const Map& emissiveMap);

	Map& getEmissiveMap() { return m_emissiveMap; }

	const Map& getEmissiveMap() const { return m_emissiveMap; }

	void setReflectiveMap(const Map& reflectiveMap);

	Map& getReflectiveMap() { return m_reflectiveMap; }

	const Map& getReflectiveMap() const { return m_reflectiveMap; }

	void setNormalMap(const Map& normalMap);

	Map& getNormalMap() { return m_normalMap; }

	const Map& getNormalMap() const { return m_normalMap; }

	void setColor(const Color4f& color);

	const Color4f& getColor() const { return m_color; }

	void setDiffuseTerm(float diffuseTerm);

	float getDiffuseTerm() const { return m_diffuseTerm; }

	void setSpecularTerm(float specularTerm);

	float getSpecularTerm() const { return m_specularTerm; }

	void setRoughness(float roughness);

	float getRoughness() const { return m_roughness; }

	void setMetalness(float metalness);

	float getMetalness() const { return m_metalness; }

	void setTransparency(float transparency);

	float getTransparency() const { return m_transparency; }

	void setEmissive(float emissive);

	float getEmissive() const { return m_emissive; }

	void setReflective(float reflective);

	float getReflective() const { return m_reflective; }

	void setBlendOperator(BlendOperator blendOperator);

	BlendOperator getBlendOperator() const { return m_blendOperator; }

	void setDoubleSided(bool doubleSided);

	bool isDoubleSided() const { return m_doubleSided; }

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name = L"Unnamed";
	Map m_diffuseMap;
	Map m_specularMap;
	Map m_roughnessMap;
	Map m_metalnessMap;
	Map m_transparencyMap;
	Map m_emissiveMap;
	Map m_reflectiveMap;
	Map m_normalMap;
	Color4f m_color = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	float m_diffuseTerm = 1.0f;
	float m_specularTerm = 0.5f;
	float m_roughness = 0.8f;
	float m_metalness = 0.0f;
	float m_transparency = 0.0f;
	float m_emissive = 0.0f;
	float m_reflective = 0.0f;
	BlendOperator m_blendOperator = BoDecal;
	bool m_doubleSided = false;
};

}
