/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_world_DecalEventInstance_H
#define traktor_world_DecalEventInstance_H

#include "World/IEntityEventInstance.h"

namespace traktor
{
	namespace world
	{

class ComponentEntity;
class DecalEvent;

/*! \brief
 * \ingroup World
 */
class DecalEventInstance : public IEntityEventInstance
{
	T_RTTI_CLASS;

public:
	DecalEventInstance(const DecalEvent* event, const Transform& Toffset);

	virtual bool update(const UpdateParams& update) override final;

	virtual void build(IWorldRenderer* worldRenderer) override final;

	virtual void cancel(CancelType when) override final;

private:
	Ref< ComponentEntity > m_entity;
};

	}
}

#endif	// traktor_world_DecalEventInstance_H
