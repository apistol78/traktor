#ifndef traktor_spray_SpawnEffectEventData_H
#define traktor_spray_SpawnEffectEventData_H

#include "World/IEntityEventData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class EntityData;

	}

	namespace spray
	{

/*! \brief
 * \ingroup Spray
 */
class T_DLLCLASS SpawnEffectEventData : public world::IEntityEventData
{
	T_RTTI_CLASS;

public:
	SpawnEffectEventData();

	virtual void serialize(ISerializer& s);

private:
	friend class EffectEntityFactory;

	Ref< world::EntityData > m_effectData;
	bool m_follow;
	bool m_useRotation;
};

	}
}

#endif	// traktor_spray_SpawnEffectEventData_H
