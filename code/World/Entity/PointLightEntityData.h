#pragma once

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

	virtual void serialize(ISerializer& s) override final;

	const Vector4& getColor() const { return m_color; }

	float getRange() const { return m_range; }

	float getRandomFlickerAmount() const { return m_randomFlickerAmount; }

	float getRandomFlickerFilter() const { return m_randomFlickerFilter; }

private:
	Vector4 m_color;
	float m_range;
	float m_randomFlickerAmount;
	float m_randomFlickerFilter;
};

	}
}
