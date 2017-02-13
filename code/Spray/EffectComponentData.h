#ifndef traktor_spray_EffectComponentData_H
#define traktor_spray_EffectComponentData_H

#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace sound
	{
	
class ISoundPlayer;

	}

	namespace world
	{
	
class IEntityEventManager;

	}

	namespace spray
	{
	
class Effect;
class EffectComponent;

/*! \brief
 * \ingroup Spray
 */
class T_DLLCLASS EffectComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< EffectComponent > createComponent(resource::IResourceManager* resourceManager, world::IEntityEventManager* eventManager, sound::ISoundPlayer* soundPlayer) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const resource::Id< Effect >& getEffect() const { return m_effect; }

private:
	resource::Id< Effect > m_effect;
};

	}
}

#endif	// traktor_spray_EffectComponentData_H
