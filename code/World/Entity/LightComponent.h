#pragma once

#include "Core/Math/Random.h"
#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

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

class SHCoeffs;

	}

	namespace world
	{

class Entity;

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS LightComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	LightComponent(
		LightType lightType,
		const Vector4& color,
		bool castShadow,
		float range,
		float radius,
		float flickerAmount,
		float flickerFilter,
		const render::SHCoeffs* shCoeffs
	);

	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void update(const UpdateParams& update) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	Transform getTransform() const;

	void setLightType(LightType lightType) { m_lightType = lightType; }

	LightType getLightType() const { return m_lightType; }

	void setColor(const Vector4& color) { m_color = color; }

	const Vector4& getColor() const { return m_color; }

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

	float getFlickerCoeff() const { return m_flickerCoeff; }

	void setSHCoeffs(const render::SHCoeffs* shCoeffs) { m_shCoeffs = shCoeffs; }

	const render::SHCoeffs* getSHCoeffs() const { return m_shCoeffs; }

private:
	Entity* m_owner;
	LightType m_lightType;
	Vector4 m_color;
	bool m_castShadow;
	float m_range;
	float m_radius;
	float m_flickerAmount;
	float m_flickerFilter;
	float m_flickerValue;
	float m_flickerCoeff;
	Random m_random;
	Ref< const render::SHCoeffs > m_shCoeffs;
};

	}
}
