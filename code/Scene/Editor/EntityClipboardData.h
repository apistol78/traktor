#ifndef traktor_scene_EntityClipboardData_H
#define traktor_scene_EntityClipboardData_H

#include "Core/Heap/Ref.h"
#include "Core/Serialization/Serializable.h"

namespace traktor
{
	namespace world
	{

class EntityData;

	}

	namespace scene
	{

class EntityClipboardData : public Serializable
{
	T_RTTI_CLASS(EntityClipboardData)

public:
	void addEntityData(world::EntityData* entityData);

	const RefArray< world::EntityData >& getEntityData() const;

	virtual bool serialize(Serializer& s);

private:
	RefArray< world::EntityData > m_entityData;
};

	}
}

#endif	// traktor_scene_EntityClipboardData_H
