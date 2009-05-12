#ifndef traktor_world_PointLightEntityData_H
#define traktor_world_PointLightEntityData_H

#include "World/Entity/SpatialEntityData.h"

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

/*! \brief Point light entity data.
 * \ingroup World
 */
class T_DLLCLASS PointLightEntityData : public SpatialEntityData
{
	T_RTTI_CLASS(PointLightEntityData)

public:
	PointLightEntityData();

	virtual bool serialize(Serializer& s);

	inline const Vector4& getSunColor() const { return m_sunColor; }

	inline const Vector4& getBaseColor() const { return m_baseColor; }

	inline const Vector4& getShadowColor() const { return m_shadowColor; }

	inline float getRange() const { return m_range; }

private:
	Vector4 m_sunColor;
	Vector4 m_baseColor;
	Vector4 m_shadowColor;
	float m_range;
};

	}
}

#endif	// traktor_world_PointLightEntityData_H
