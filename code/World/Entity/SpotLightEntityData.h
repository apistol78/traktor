#ifndef traktor_world_SpotLightEntityData_H
#define traktor_world_SpotLightEntityData_H

#include "World/EntityData.h"

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

/*! \brief Spot light entity data.
 * \ingroup World
 */
class T_DLLCLASS SpotLightEntityData : public EntityData
{
	T_RTTI_CLASS;

public:
	SpotLightEntityData();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const Vector4& getSunColor() const { return m_sunColor; }

	const Vector4& getBaseColor() const { return m_baseColor; }

	const Vector4& getShadowColor() const { return m_shadowColor; }

	float getRange() const { return m_range; }

	float getRadius() const { return m_radius; }

	bool getCastShadow() const { return m_castShadow; }

private:
	Vector4 m_sunColor;
	Vector4 m_baseColor;
	Vector4 m_shadowColor;
	float m_range;
	float m_radius;
	bool m_castShadow;
};

	}
}

#endif	// traktor_world_SpotLightEntityData_H
