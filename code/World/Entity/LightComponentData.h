#pragma once

#include "Core/Math/Color4f.h"
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

	virtual void serialize(ISerializer& s) override final;

	void setLightType(LightType lightType) { m_lightType = lightType; }

	LightType getLightType() const { return m_lightType; }

	void setColor(const Color4f& color) { m_color = color; }

	const Color4f& getColor() const { return m_color; }

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
	Color4f m_color;
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
