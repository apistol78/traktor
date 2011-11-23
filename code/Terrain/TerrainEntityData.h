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
class MaterialMask;

	}

	namespace terrain
	{

class TerrainSurface;

class T_DLLCLASS TerrainEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	enum VisualizeMode
	{
		VmDefault,
		VmSurfaceLod,
		VmPatchLod
	};

	TerrainEntityData();

	virtual bool serialize(ISerializer& s);

	const resource::Proxy< hf::Heightfield >& getHeightfield() const { return m_heightfield; }

	const resource::Proxy< hf::MaterialMask >& getMaterialMask() const { return m_materialMask; }

	TerrainSurface* getSurface() const { return m_surface; }

	float getPatchLodDistance() const { return m_patchLodDistance; }

	float getPatchLodBias() const { return m_patchLodBias; }

	float getPatchLodExponent() const { return m_patchLodExponent; }

	float getSurfaceLodDistance() const { return m_surfaceLodDistance; }

	float getSurfaceLodBias() const { return m_surfaceLodBias; }

	float getSurfaceLodExponent() const { return m_surfaceLodExponent; }

	VisualizeMode getVisualizeMode() const { return m_visualizeMode; }

private:
	resource::Proxy< hf::Heightfield > m_heightfield;
	resource::Proxy< hf::MaterialMask > m_materialMask;
	Ref< TerrainSurface > m_surface;
	float m_patchLodDistance;
	float m_patchLodBias;
	float m_patchLodExponent;
	float m_surfaceLodDistance;
	float m_surfaceLodBias;
	float m_surfaceLodExponent;
	VisualizeMode m_visualizeMode;
};

	}
}

#endif	// traktor_terrain_TerrainEntityData_H
