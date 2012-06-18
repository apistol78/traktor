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

	Material();

	Material(const std::wstring& name);

	void setName(const std::wstring& name);
	
	const std::wstring& getName() const;

	void setDiffuseMap(const std::wstring& diffuseMap);

	const std::wstring& getDiffuseMap() const;

	void setSpecularMap(const std::wstring& specularMap);

	const std::wstring& getSpecularMap() const;

	void setEmissiveMap(const std::wstring& emissiveMap);

	const std::wstring& getReflectiveMap() const;

	void setReflectiveMap(const std::wstring& reflectiveMap);

	const std::wstring& getEmissiveMap() const;

	void setNormalMap(const std::wstring& normalMap);

	const std::wstring& getNormalMap() const;

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
	std::wstring m_diffuseMap;
	std::wstring m_specularMap;
	std::wstring m_emissiveMap;
	std::wstring m_reflectiveMap;
	std::wstring m_normalMap;
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
