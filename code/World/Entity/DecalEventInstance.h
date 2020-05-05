#pragma once

#include "World/IEntityEventInstance.h"

namespace traktor
{
	namespace world
	{

class Entity;
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

	virtual void gather(const std::function< void(Entity*) >& fn) const override final;

	virtual void cancel(Cancel when) override final;

private:
	Ref< Entity > m_entity;
};

	}
}

