#ifndef traktor_mesh_StreamMeshEntity_H
#define traktor_mesh_StreamMeshEntity_H

#include "Resource/Proxy.h"
#include "Mesh/MeshEntity.h"
#include "Mesh/Stream/StreamMesh.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class T_DLLCLASS StreamMeshEntity : public MeshEntity
{
	T_RTTI_CLASS;

public:
	StreamMeshEntity(const Transform& transform, const resource::Proxy< StreamMesh >& mesh);

	uint32_t getFrameCount() const;

	void setFrame(uint32_t frame);
	
	virtual Aabb3 getBoundingBox() const;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		float distance
	);

private:
	mutable resource::Proxy< StreamMesh > m_mesh;
	mutable Ref< StreamMesh::Instance > m_instance;
	uint32_t m_frame;

	bool validate() const;
};

	}
}

#endif	// traktor_mesh_StreamMeshEntity_H
