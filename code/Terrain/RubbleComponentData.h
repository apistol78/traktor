#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Resource/Id.h"
#include "Terrain/TerrainLayerComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

class InstanceMesh;

}

namespace traktor::terrain
{

class T_DLLCLASS RubbleComponentData : public TerrainLayerComponentData
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

private:
	friend class RubbleComponent;
	friend class TerrainEntityPipeline;

	struct RubbleMesh
	{
		resource::Id< mesh::InstanceMesh > mesh;
		uint8_t attribute = 1;
		int32_t density = 10;
		float randomScaleAmount = 0.5f;
		float randomTilt = 0.0f;
		float upness = 1.0f;

		void serialize(ISerializer& s);
	};

	float m_spreadDistance = 100.0f;
	AlignedVector< RubbleMesh > m_rubble;
};

}
