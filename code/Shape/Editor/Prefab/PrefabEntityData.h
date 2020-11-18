#pragma once

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

	virtual void serialize(ISerializer& s) override final;

	bool partitionMesh() const { return m_partitionMesh; }

private:
	bool m_partitionMesh;
};

	}
}

