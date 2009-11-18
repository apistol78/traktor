#ifndef traktor_mesh_StaticMeshResource_H
#define traktor_mesh_StaticMeshResource_H

#include "Mesh/MeshResource.h"
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

class T_DLLCLASS StaticMeshResource : public MeshResource
{
	T_RTTI_CLASS;

public:
	struct Part
	{
		std::wstring name;
		Guid material;

		bool serialize(ISerializer& s);
	};

	void setParts(const std::vector< Part >& parts);

	const std::vector< Part >& getParts() const;

	virtual bool serialize(ISerializer& s);

private:
	std::vector< Part > m_parts;
};

	}
}

#endif	// traktor_mesh_StaticMeshResource_H
