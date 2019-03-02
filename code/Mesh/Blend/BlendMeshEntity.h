#pragma once

#include "Mesh/MeshEntity.h"
#include "Mesh/Blend/BlendMesh.h"
#include "Resource/Proxy.h"

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

/*! \brief Blend mesh entity
 * \ingroup Mesh
 */
class T_DLLCLASS BlendMeshEntity : public MeshEntity
{
	T_RTTI_CLASS;

public:
	BlendMeshEntity(const Transform& transform, bool screenSpaceCulling, const resource::Proxy< BlendMesh >& mesh);

	void setBlendWeights(const AlignedVector< float >& blendWeights);

	const AlignedVector< float >& getBlendWeights() const;

	virtual Aabb3 getBoundingBox() const override final;

	virtual bool supportTechnique(render::handle_t technique) const override final;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		float distance
	) override final;

private:
	resource::Proxy< BlendMesh > m_mesh;
	Ref< BlendMesh::Instance > m_instance;
	AlignedVector< float > m_blendWeights;
};

	}
}

