#ifndef traktor_scene_EntityClipboardData_H
#define traktor_scene_EntityClipboardData_H

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace world
	{

class EntityInstance;

	}

	namespace scene
	{

class EntityClipboardData : public ISerializable
{
	T_RTTI_CLASS;

public:
	void addInstance(world::EntityInstance* instance);

	const RefArray< world::EntityInstance >& getInstances() const;

	virtual bool serialize(ISerializer& s);

private:
	RefArray< world::EntityInstance > m_instances;
};

	}
}

#endif	// traktor_scene_EntityClipboardData_H
