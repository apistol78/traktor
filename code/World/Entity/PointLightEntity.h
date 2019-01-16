#pragma once

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
		const Vector4& color,
		float range,
		float randomFlickerAmount,
		float randomFlickerFilter
	);

	virtual void update(const UpdateParams& update) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual bool getTransform(Transform& outTransform) const override final;

	virtual Aabb3 getBoundingBox() const override final;

	const Vector4& getColor() const { return m_color; }

	float getRange() const { return m_range; }

	float getRandomFlicker() const { return m_randomFlicker; }

private:
	Transform m_transform;
	Vector4 m_color;
	float m_range;
	float m_randomFlickerAmount;
	float m_randomFlickerFilter;
	float m_randomFlickerValue;
	float m_randomFlicker;
	Random m_random;
};

	}
}
