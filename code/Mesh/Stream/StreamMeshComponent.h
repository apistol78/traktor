#pragma once

#include "Mesh/MeshComponent.h"
#include "Mesh/Stream/StreamMesh.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

/*! Streaming mesh component.
 * \ingroup Mesh
 */
class T_DLLCLASS StreamMeshComponent : public MeshComponent
{
	T_RTTI_CLASS;

public:
	explicit StreamMeshComponent(const resource::Proxy< StreamMesh >& mesh, bool screenSpaceCulling);

	virtual void destroy() override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass) override final;

	uint32_t getFrameCount() const;

	void setFrame(uint32_t frame);

	uint32_t getFrame() const;

private:
	resource::Proxy< StreamMesh > m_mesh;
	Ref< StreamMesh::Instance > m_instance;
	uint32_t m_frame;
};

}
