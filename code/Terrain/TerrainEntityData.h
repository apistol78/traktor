#ifndef traktor_terrain_TerrainEntityData_H
#define traktor_terrain_TerrainEntityData_H

#include "World/Entity/EntityData.h"
#include "Resource/Proxy.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Shader;

	}

	namespace terrain
	{

class Heightfield;
class TerrainSurface;

class T_DLLCLASS TerrainEntityData : public world::EntityData
{
	T_RTTI_CLASS(TerrainEntityData)

public:
	TerrainEntityData();

	virtual bool serialize(Serializer& s);

	inline const resource::Proxy< Heightfield >& getHeightfield() const { return m_heightfield; }

	inline const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

	inline Ref< TerrainSurface > getSurface() const { return m_surface; }

private:
	friend class TerrainEntity;

	resource::Proxy< Heightfield > m_heightfield;
	resource::Proxy< render::Shader > m_shader;
	Ref< TerrainSurface > m_surface;
	float m_patchLodDistance;
	float m_surfaceLodDistance;
};

	}
}

#endif	// traktor_terrain_TerrainEntityData_H
