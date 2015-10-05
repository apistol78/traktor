#ifndef traktor_world_DecalEventInstance_H
#define traktor_world_DecalEventInstance_H

#include "World/IEntityEventInstance.h"

namespace traktor
{
	namespace world
	{

class DecalEntity;
class DecalEvent;

/*! \brief
 * \ingroup World
 */
class DecalEventInstance : public IEntityEventInstance
{
	T_RTTI_CLASS;

public:
	DecalEventInstance(const DecalEvent* event, const Transform& Toffset);

	virtual bool update(const UpdateParams& update) T_OVERRIDE T_FINAL;

	virtual void build(IWorldRenderer* worldRenderer) T_OVERRIDE T_FINAL;

	virtual void cancel(CancelType when) T_OVERRIDE T_FINAL;

private:
	Ref< DecalEntity > m_entity;
};

	}
}

#endif	// traktor_world_DecalEventInstance_H
