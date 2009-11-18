#ifndef traktor_mesh_IndoorMeshResource_H
#define traktor_mesh_IndoorMeshResource_H

#include "Mesh/MeshResource.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb.h"
#include "Core/Guid.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class T_DLLCLASS IndoorMeshResource : public MeshResource
{
	T_RTTI_CLASS;

public:
	struct Part
	{
		Guid material;
		int32_t meshPart;

		bool serialize(ISerializer& s);
	};

	struct Sector
	{
		Vector4 min;
		Vector4 max;
		std::vector< Part > parts;

		bool serialize(ISerializer& s);
	};

	struct Portal
	{
		AlignedVector< Vector4 > pts;
		int32_t sectorA;
		int32_t sectorB;

		bool serialize(ISerializer& s);
	};

	void setSectors(const AlignedVector< Sector >& sectors);

	const AlignedVector< Sector >& getSectors() const;

	void setPortals(const AlignedVector< Portal >& portals);

	const AlignedVector< Portal >& getPortals() const;

	virtual bool serialize(ISerializer& s);

private:
	AlignedVector< Sector > m_sectors;
	AlignedVector< Portal > m_portals;
};

	}
}

#endif	// traktor_mesh_IndoorMeshResource_H
