#pragma once

#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::terrain
{

class T_DLLCLASS TerrainLayerComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override;
};

}

