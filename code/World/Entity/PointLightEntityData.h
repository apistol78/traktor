#ifndef traktor_world_PointLightEntityData_H
#define traktor_world_PointLightEntityData_H

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

/*! \brief Point light entity data.
 * \ingroup World
 */
class T_DLLCLASS PointLightEntityData : public EntityData
{
	T_RTTI_CLASS;

public:
	PointLightEntityData();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const Vector4& getSunColor() const { return m_sunColor; }

	const Vector4& getBaseColor() const { return m_baseColor; }

	const Vector4& getShadowColor() const { return m_shadowColor; }

	float getRange() const { return m_range; }

	float getRandomFlickerAmount() const { return m_randomFlickerAmount; }

	float getRandomFlickerFilter() const { return m_randomFlickerFilter; }

private:
	Vector4 m_sunColor;
	Vector4 m_baseColor;
	Vector4 m_shadowColor;
	float m_range;
	float m_randomFlickerAmount;
	float m_randomFlickerFilter;
};

	}
}

#endif	// traktor_world_PointLightEntityData_H
