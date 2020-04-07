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

namespace traktor
{
	namespace mesh
	{

class InstanceMesh;

	}

	namespace terrain
	{

class T_DLLCLASS RubbleComponentData : public TerrainLayerComponentData
{
	T_RTTI_CLASS;

public:
	RubbleComponentData();

	virtual void serialize(ISerializer& s) override final;

private:
	friend class RubbleComponent;
	friend class TerrainEntityPipeline;

	struct RubbleMesh
	{
		resource::Id< mesh::InstanceMesh > mesh;
		uint8_t attribute;
		int32_t density;
		float randomScaleAmount;

		RubbleMesh();

		void serialize(ISerializer& s);
	};

	float m_spreadDistance;
	AlignedVector< RubbleMesh > m_rubble;
};

	}
}

