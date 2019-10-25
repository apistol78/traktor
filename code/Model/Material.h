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

namespace traktor
{
	namespace model
	{

/*! \brief Material descriptor.
 * \ingroup Model
 */
class T_DLLCLASS Material : public PropertyGroup
{
	T_RTTI_CLASS;

public:
	enum BlendOperator
	{
		BoDecal,
		BoAdd,
		BoMultiply,
		BoAlpha
	};

	struct Map
	{
		std::wstring name;
		uint32_t channel;
		bool anisotropic;
		Guid texture;

		Map()
		:	name(L"")
		,	channel(0)
		,	anisotropic(false)
		{
		}

		Map(const std::wstring& name_, uint32_t channel_, bool anisotropic_ = true, const Guid& texture_ = Guid())
		:	name(name_)
		,	channel(channel_)
		,	anisotropic(anisotropic_)
		,	texture(texture_)
		{
		}

		void serialize(ISerializer& s);
	};

	Material();

	explicit Material(const std::wstring& name);

	explicit Material(const std::wstring& name, const Color4f& color);

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	void setDiffuseMap(const Map& diffuseMap);

	const Map& getDiffuseMap() const;

	void setSpecularMap(const Map& specularMap);

	const Map& getSpecularMap() const;

	void setRoughnessMap(const Map& roughnessMap);

	const Map& getRoughnessMap() const;

	void setMetalnessMap(const Map& metalnessMap);

	const Map& getMetalnessMap() const;

	void setTransparencyMap(const Map& transparencyMap);

	const Map& getTransparencyMap() const;

	void setEmissiveMap(const Map& emissiveMap);

	const Map& getEmissiveMap() const;

	void setReflectiveMap(const Map& reflectiveMap);

	const Map& getReflectiveMap() const;

	void setNormalMap(const Map& normalMap);

	const Map& getNormalMap() const;

	void setLightMap(const Map& lightMap);

	const Map& getLightMap() const;

	void setColor(const Color4f& color);

	const Color4f& getColor() const;

	void setDiffuseTerm(float diffuseTerm);

	float getDiffuseTerm() const;

	void setSpecularTerm(float specularTerm);

	float getSpecularTerm() const;

	void setRoughness(float roughness);

	float getRoughness() const;

	void setMetalness(float metalness);

	float getMetalness() const;

	void setTransparency(float transparency);

	float getTransparency() const;

	void setEmissive(float emissive);

	float getEmissive() const;

	void setReflective(float reflective);

	float getReflective() const;

	void setRimLightIntensity(float rimLightIntensity);

	float getRimLightIntensity() const;

	void setBlendOperator(BlendOperator blendOperator);

	BlendOperator getBlendOperator() const;

	void setDoubleSided(bool doubleSided);

	bool isDoubleSided() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
	Map m_diffuseMap;
	Map m_specularMap;
	Map m_roughnessMap;
	Map m_metalnessMap;
	Map m_transparencyMap;
	Map m_emissiveMap;
	Map m_reflectiveMap;
	Map m_normalMap;
	Map m_lightMap;
	uint32_t m_lightMapFlags;
	Color4f m_color;
	float m_diffuseTerm;
	float m_specularTerm;
	float m_roughness;
	float m_metalness;
	float m_transparency;
	float m_emissive;
	float m_reflective;
	float m_rimLightIntensity;
	BlendOperator m_blendOperator;
	bool m_doubleSided;
};

	}
}

