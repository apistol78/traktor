#pragma once

#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "World/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace shape
	{

class T_DLLCLASS PrefabEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	PrefabEntityData();

	void addEntityData(world::EntityData* entityData);

	void removeEntityData(world::EntityData* entityData);

	void removeAllEntityData();

	virtual void serialize(ISerializer& s) override final;

	virtual void setTransform(const Transform& transform) override final;

	const RefArray< world::EntityData >& getEntityData() const { return m_entityData; }

	bool partitionMesh() const { return m_partitionMesh; }

private:
	RefArray< world::EntityData > m_entityData;
	bool m_partitionMesh;
};

	}
}

