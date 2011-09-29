#ifndef traktor_terrain_TerrainEntityData_H
#define traktor_terrain_TerrainEntityData_H

#include "Core/Math/Vector4.h"
#include "Resource/Proxy.h"
#include "World/Entity/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class Heightfield;

	}

	namespace render
	{

class Shader;

	}

	namespace terrain
	{

class TerrainSurface;

class T_DLLCLASS TerrainEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	TerrainEntityData();

	virtual bool serialize(ISerializer& s);

	const resource::Proxy< hf::Heightfield >& getHeightfield() const { return m_heightfield; }

	const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

	Ref< TerrainSurface > getSurface() const { return m_surface; }

private:
	friend class TerrainEntity;

	resource::Proxy< hf::Heightfield > m_heightfield;
	resource::Proxy< render::Shader > m_shader;
	Ref< TerrainSurface > m_surface;
	float m_patchLodDistance;
	float m_surfaceLodDistance;
};

	}
}

#endif	// traktor_terrain_TerrainEntityData_H
