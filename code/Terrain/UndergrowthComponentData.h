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
	namespace terrain
	{

class T_DLLCLASS UndergrowthComponentData : public TerrainLayerComponentData
{
	T_RTTI_CLASS;

public:
	UndergrowthComponentData();

	virtual void serialize(ISerializer& s) override final;

private:
	friend class UndergrowthComponent;
	friend class TerrainEntityPipeline;

	struct Plant
	{
		uint8_t material;
		int32_t density;
		int32_t plant;
		float scale;

		Plant();

		void serialize(ISerializer& s);
	};

	resource::Id< render::Shader > m_shader;
	float m_spreadDistance;
	AlignedVector< Plant > m_plants;
};

	}
}

