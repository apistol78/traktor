#ifndef traktor_model_Material_H
#define traktor_model_Material_H

#include "Core/Config.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Math/Color4ub.h"

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

		Map()
		:	name(L"")
		,	channel(0)
		,	anisotropic(false)
		{
		}

		Map(const std::wstring& name_, uint32_t channel_, bool anisotropic_)
		:	name(name_)
		,	channel(channel_)
		,	anisotropic(anisotropic_)
		{
		}
	};

	Material();

	Material(const std::wstring& name);

	void setName(const std::wstring& name);
	
	const std::wstring& getName() const;

	void setDiffuseMap(const Map& diffuseMap);

	const Map& getDiffuseMap() const;

	void setSpecularMap(const Map& specularMap);

	const Map& getSpecularMap() const;

	void setEmissiveMap(const Map& emissiveMap);

	const Map& getEmissiveMap() const;

	void setReflectiveMap(const Map& reflectiveMap);

	const Map& getReflectiveMap() const;

	void setNormalMap(const Map& normalMap);

	const Map& getNormalMap() const;

	void setColor(const Color4ub& color);

	const Color4ub& getColor() const;

	void setDiffuseTerm(float diffuseTerm);

	float getDiffuseTerm() const;

	void setSpecularTerm(float specularTerm);

	float getSpecularTerm() const;

	void setSpecularRoughness(float specularRoughness);

	float getSpecularRoughness() const;

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
	
private:
	std::wstring m_name;
	Map m_diffuseMap;
	Map m_specularMap;
	Map m_emissiveMap;
	Map m_reflectiveMap;
	Map m_normalMap;
	Color4ub m_color;
	float m_diffuseTerm;
	float m_specularTerm;
	float m_specularRoughness;
	float m_emissive;
	float m_reflective;
	float m_rimLightIntensity;
	BlendOperator m_blendOperator;
	bool m_doubleSided;
};

	}
}

#endif	// traktor_model_Material_H
