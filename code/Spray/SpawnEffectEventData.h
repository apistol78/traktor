#ifndef traktor_spray_SpawnEffectEventData_H
#define traktor_spray_SpawnEffectEventData_H

#include "Resource/Id.h"
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
	namespace spray
	{

class Effect;

/*! \brief
 * \ingroup Spray
 */
class T_DLLCLASS SpawnEffectEventData : public world::IEntityEventData
{
	T_RTTI_CLASS;

public:
	SpawnEffectEventData();

	virtual void serialize(ISerializer& s);

	const resource::Id< Effect >& getEffect() const { return m_effect; }

	bool getFollow() const { return m_follow; }

	bool getUseRotation() const { return m_useRotation; }

private:
	resource::Id< Effect > m_effect;
	bool m_follow;
	bool m_useRotation;
};

	}
}

#endif	// traktor_spray_SpawnEffectEventData_H
