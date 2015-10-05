#ifndef traktor_world_PointLightEntity_H
#define traktor_world_PointLightEntity_H

#include "Core/Math/Random.h"
#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Point light entity.
 * \ingroup World
 */
class T_DLLCLASS PointLightEntity : public Entity
{
	T_RTTI_CLASS;

public:
	PointLightEntity(
		const Transform& transform,
		const Vector4& sunColor,
		const Vector4& baseColor,
		const Vector4& shadowColor,
		float range,
		float randomFlickerAmount,
		float randomFlickerFilter
	);

	virtual void update(const UpdateParams& update) T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual bool getTransform(Transform& outTransform) const T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	const Vector4& getSunColor() const { return m_sunColor; }

	const Vector4& getBaseColor() const { return m_baseColor; }

	const Vector4& getShadowColor() const { return m_shadowColor; }

	float getRange() const { return m_range; }

	float getRandomFlicker() const { return m_randomFlicker; }

private:
	Transform m_transform;
	Vector4 m_sunColor;
	Vector4 m_baseColor;
	Vector4 m_shadowColor;
	float m_range;
	float m_randomFlickerAmount;
	float m_randomFlickerFilter;
	float m_randomFlickerValue;
	float m_randomFlicker;
	Random m_random;
};

	}
}

#endif	// traktor_world_PointLightEntity_H
