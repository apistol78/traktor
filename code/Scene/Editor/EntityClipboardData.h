#ifndef traktor_scene_EntityClipboardData_H
#define traktor_scene_EntityClipboardData_H

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace world
	{

class EntityData;

	}

	namespace scene
	{

class EntityClipboardData : public ISerializable
{
	T_RTTI_CLASS;

public:
	void addEntityData(world::EntityData* entityData);

	const RefArray< world::EntityData >& getEntityData() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	RefArray< world::EntityData > m_entityData;
};

	}
}

#endif	// traktor_scene_EntityClipboardData_H
