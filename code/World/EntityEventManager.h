/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	virtual IEntityEventInstance* raise(const IEntityEvent* event, Entity* sender, const Transform& Toffset) override final;

	virtual IEntityEventInstance* raise(const EntityEventSet* eventSet, const std::wstring& eventId, Entity* sender, const Transform& Toffset) override final;

	virtual void update(const UpdateParams& update) override final;

	virtual void build(IWorldRenderer* worldRenderer) override final;

	virtual void cancelAll(CancelType when) override final;

private:
	uint32_t m_maxEventInstances;
	RefArray< IEntityEventInstance > m_eventInstances;
	Semaphore m_lock;
};

	}
}

#endif	// traktor_world_EntityEventManager_H
