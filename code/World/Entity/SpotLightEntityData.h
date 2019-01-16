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

/*! \brief Spot light entity data.
 * \ingroup World
 */
class T_DLLCLASS SpotLightEntityData : public EntityData
{
	T_RTTI_CLASS;

public:
	SpotLightEntityData();

	virtual void serialize(ISerializer& s) override final;

	const Vector4& getColor() const { return m_color; }

	float getRange() const { return m_range; }

	float getRadius() const { return m_radius; }

	bool getCastShadow() const { return m_castShadow; }

private:
	Vector4 m_color;
	float m_range;
	float m_radius;
	bool m_castShadow;
};

	}
}
