#ifndef traktor_world_PointLightEntity_H
#define traktor_world_PointLightEntity_H

#include "World/Entity/SpatialEntity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Point light entity.
 * \ingroup World
 */
class T_DLLCLASS PointLightEntity : public SpatialEntity
{
	T_RTTI_CLASS(PointLightEntity)

public:
	PointLightEntity(
		const Transform& transform,
		const Vector4& sunColor,
		const Vector4& baseColor,
		const Vector4& shadowColor,
		float range
	);

	virtual void update(const EntityUpdate* update);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb getBoundingBox() const;

	inline const Vector4& getSunColor() const { return m_sunColor; }

	inline const Vector4& getBaseColor() const { return m_baseColor; }

	inline const Vector4& getShadowColor() const { return m_shadowColor; }

	inline float getRange() const { return m_range; }

private:
	Transform m_transform;
	Vector4 m_sunColor;
	Vector4 m_baseColor;
	Vector4 m_shadowColor;
	float m_range;
};

	}
}

#endif	// traktor_world_PointLightEntity_H
