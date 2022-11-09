#pragma once

#include <functional>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Transform.h"
#include "Core/Thread/Semaphore.h"
#include "World/WorldTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class Entity;
class IEntityEvent;
class IEntityEventInstance;

struct UpdateParams;

/*!
 * \ingroup World
 */
class T_DLLCLASS EntityEventManager : public Object
{
	T_RTTI_CLASS;

public:
	explicit EntityEventManager(uint32_t maxEventsInstances);

	IEntityEventInstance* raise(const IEntityEvent* event, Entity* sender, const Transform& Toffset);

	void update(const UpdateParams& update);

	void gather(const std::function< void(Entity*) >& fn) const;

	void cancelAll(Cancel when);

private:
	uint32_t m_maxEventInstances;
	RefArray< IEntityEventInstance > m_eventInstances;
	mutable Semaphore m_lock;
};

}
