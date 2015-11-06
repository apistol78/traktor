#ifndef traktor_world_EntityEventManager_H
#define traktor_world_EntityEventManager_H

#include "Core/RefArray.h"
#include "Core/Thread/Semaphore.h"
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

	virtual IEntityEventInstance* raise(const IEntityEvent* event, Entity* sender, const Transform& Toffset) T_OVERRIDE T_FINAL;

	virtual IEntityEventInstance* raise(const EntityEventSet* eventSet, const std::wstring& eventId, Entity* sender, const Transform& Toffset) T_OVERRIDE T_FINAL;

	virtual void update(const UpdateParams& update) T_OVERRIDE T_FINAL;

	virtual void build(IWorldRenderer* worldRenderer) T_OVERRIDE T_FINAL;

	virtual void cancelAll(CancelType when) T_OVERRIDE T_FINAL;

private:
	uint32_t m_maxEventInstances;
	RefArray< IEntityEventInstance > m_eventInstances;
	Semaphore m_lock;
};

	}
}

#endif	// traktor_world_EntityEventManager_H
