/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_world_LightComponentData_H
#define traktor_world_LightComponentData_H

#include "Core/Math/Vector4.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"
#include "World/WorldTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ITexture;

	}

	namespace world
	{

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS LightComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	LightComponentData();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	void setLightType(LightType lightType) { m_lightType = lightType; }

	LightType getLightType() const { return m_lightType; }

	void setSunColor(const Vector4& sunColor) { m_sunColor = sunColor; }

	const Vector4& getSunColor() const { return m_sunColor; }

	void setBaseColor(const Vector4& baseColor) { m_baseColor = baseColor; }

	const Vector4& getBaseColor() const { return m_baseColor; }

	void setShadowColor(const Vector4& shadowColor) { m_shadowColor = shadowColor; }

	const Vector4& getShadowColor() const { return m_shadowColor; }

	void setProbeDiffuseTexture(const resource::Id< render::ITexture >& probeTexture) { m_probeDiffuseTexture = probeTexture; }

	const resource::Id< render::ITexture >& getProbeDiffuseTexture() const { return m_probeDiffuseTexture; }

	void setProbeSpecularTexture(const resource::Id< render::ITexture >& probeTexture) { m_probeSpecularTexture = probeTexture; }

	const resource::Id< render::ITexture >& getProbeSpecularTexture() const { return m_probeSpecularTexture; }

	void setCloudShadowTexture(const resource::Id< render::ITexture >& cloudShadowTexture) { m_cloudShadowTexture = cloudShadowTexture; }

	const resource::Id< render::ITexture >& getCloudShadowTexture() const { return m_cloudShadowTexture; }

	void setCastShadow(bool castShadow) { m_castShadow = castShadow; }

	bool getCastShadow() const { return m_castShadow; }

	void setRange(float range) { m_range = range;  }

	float getRange() const { return m_range; }

	void setRadius(float radius) { m_radius = radius; }

	float getRadius() const { return m_radius;  }

	void setFlickerAmount(float flickerAmount) { m_flickerAmount = flickerAmount; }

	float getFlickerAmount() const { return m_flickerAmount; }

	void setFlickerFilter(float flickerFilter) { m_flickerFilter = flickerFilter; }

	float getFlickerFilter() const { return m_flickerFilter; }

private:
	LightType m_lightType;
	Vector4 m_sunColor;
	Vector4 m_baseColor;
	Vector4 m_shadowColor;
	resource::Id< render::ITexture > m_probeDiffuseTexture;
	resource::Id< render::ITexture > m_probeSpecularTexture;
	resource::Id< render::ITexture > m_cloudShadowTexture;
	bool m_castShadow;
	float m_range;
	float m_radius;
	float m_flickerAmount;
	float m_flickerFilter;
};

	}
}

#endif	// traktor_world_LightComponentData_H
