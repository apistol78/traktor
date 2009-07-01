#ifndef traktor_scene_EntityClipboardData_H
#define traktor_scene_EntityClipboardData_H

#include "Core/Heap/Ref.h"
#include "Core/Serialization/Serializable.h"

namespace traktor
{
	namespace world
	{

class EntityInstance;

	}

	namespace scene
	{

class EntityClipboardData : public Serializable
{
	T_RTTI_CLASS(EntityClipboardData)

public:
	void addInstance(world::EntityInstance* instance);

	const RefArray< world::EntityInstance >& getInstances() const;

	virtual bool serialize(Serializer& s);

private:
	RefArray< world::EntityInstance > m_instances;
};

	}
}

#endif	// traktor_scene_EntityClipboardData_H
