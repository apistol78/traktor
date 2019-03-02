#pragma once

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

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< world::EntityData > m_entityData;
};

	}
}

