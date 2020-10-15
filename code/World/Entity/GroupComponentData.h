#pragma once

#include "Core/RefArray.h"
#include "World/IEntityComponentData.h"

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

class EntityData;

/*! Group component persistent data.
 * \ingroup World
 */
class T_DLLCLASS GroupComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	GroupComponentData() = default;

	explicit GroupComponentData(const RefArray< EntityData >& entityData);

	void addEntityData(EntityData* entityData);

	void removeEntityData(EntityData* entityData);

	void removeAllEntityData();

	void setEntityData(const RefArray< EntityData >& entityData);

	RefArray< EntityData >& getEntityData();

	const RefArray< EntityData >& getEntityData() const;

	virtual void setTransform(const EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

private:
    RefArray< EntityData > m_entityData;
};

	}
}
