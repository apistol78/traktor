#ifndef traktor_mesh_InstanceMeshEntityRenderer_H
#define traktor_mesh_InstanceMeshEntityRenderer_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "World/Entity/EntityRenderer.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshData.h"

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

class T_DLLCLASS InstanceMeshEntityRenderer : public world::EntityRenderer
{
	T_RTTI_CLASS(InstanceMeshEntityRenderer)

public:
	virtual const TypeSet getEntityTypes() const;

	virtual void render(
		world::WorldContext* worldContext,
		world::WorldRenderView* worldRenderView,
		world::Entity* entity
	);

	virtual void flush(
		world::WorldContext* worldContext,
		world::WorldRenderView* worldRenderView
	);

private:
	std::map< InstanceMesh*, AlignedVector< InstanceMesh::instance_distance_t > > m_meshInstances;
};

	}
}

#endif	// traktor_mesh_InstanceMeshEntityRenderer_H
