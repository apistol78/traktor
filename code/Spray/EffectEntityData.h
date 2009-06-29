#ifndef traktor_spray_EffectEntityData_H
#define traktor_spray_EffectEntityData_H

#include "Resource/Proxy.h"
#include "World/Entity/SpatialEntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace spray
	{

class EffectEntity;
class Effect;

/*! \brief Effect entity data.
 * \ingroup Spray
 */
class T_DLLCLASS EffectEntityData : public world::SpatialEntityData
{
	T_RTTI_CLASS(EffectEntityData)

public:
	EffectEntity* createEntity(resource::IResourceManager* resourceManager) const;

	virtual bool serialize(Serializer& s);

	inline const resource::Proxy< Effect >& getEffect() const { return m_effect; }

private:
	mutable resource::Proxy< Effect > m_effect;
};

	}
}

#endif	// traktor_spray_EffectEntityData_H
