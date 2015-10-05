#ifndef traktor_world_SpotLightEntity_H
#define traktor_world_SpotLightEntity_H

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

/*! \brief Spot light entity.
 * \ingroup World
 */
class T_DLLCLASS SpotLightEntity : public Entity
{
	T_RTTI_CLASS;

public:
	SpotLightEntity(
		const Transform& transform,
		const Vector4& sunColor,
		const Vector4& baseColor,
		const Vector4& shadowColor,
		float range,
		float radius,
		bool castShadow
	);

	virtual void update(const UpdateParams& update) T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual bool getTransform(Transform& outTransform) const T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	const Vector4& getSunColor() const { return m_sunColor; }

	const Vector4& getBaseColor() const { return m_baseColor; }

	const Vector4& getShadowColor() const { return m_shadowColor; }

	float getRange() const { return m_range; }

	float getRadius() const { return m_radius; }

	void setCastShadow(bool castShadow) { m_castShadow = castShadow; }

	bool getCastShadow() const { return m_castShadow; }

private:
	Transform m_transform;
	Vector4 m_sunColor;
	Vector4 m_baseColor;
	Vector4 m_shadowColor;
	float m_range;
	float m_radius;
	bool m_castShadow;
};

	}
}

#endif	// traktor_world_SpotLightEntity_H
