#ifndef traktor_world_EntityEventManager_H
#define traktor_world_EntityEventManager_H

#include "Core/RefArray.h"
#include "World/IEntityEventManager.h"

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

class IEntityEventInstance;

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS EntityEventManager : public IEntityEventManager
{
	T_RTTI_CLASS;

public:
	EntityEventManager(uint32_t maxEventsInstances);

	virtual void raise(const IEntityEvent* event, Entity* sender, const Transform& Toffset);

	virtual void raise(const EntityEventSet* eventSet, const std::wstring& eventId, Entity* sender, const Transform& Toffset);

	virtual void update(const UpdateParams& update);

	virtual void build(IWorldRenderer* worldRenderer);

private:
	uint32_t m_maxEventInstances;
	RefArray< IEntityEventInstance > m_eventInstances;
};

	}
}

#endif	// traktor_world_EntityEventManager_H
