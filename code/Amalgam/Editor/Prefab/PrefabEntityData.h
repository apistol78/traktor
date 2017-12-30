#ifndef traktor_amalgam_PrefabEntityData_H
#define traktor_amalgam_PrefabEntityData_H

#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "World/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class T_DLLCLASS PrefabEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	PrefabEntityData();

	void addEntityData(world::EntityData* entityData);

	void removeEntityData(world::EntityData* entityData);

	void removeAllEntityData();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;
	
	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	const Guid& getOutputGuid(int32_t index) const { return m_outputGuid[index]; }

	const RefArray< world::EntityData >& getEntityData() const { return m_entityData; }

	bool partitionMesh() const { return m_partitionMesh; }

private:
	Guid m_outputGuid[2];
	RefArray< world::EntityData > m_entityData;
	bool m_partitionMesh;
};

	}
}

#endif	// traktor_amalgam_PrefabEntityData_H
