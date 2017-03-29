#ifndef traktor_mesh_AutoLodMesh_H
#define traktor_mesh_AutoLodMesh_H

#include "Core/RefArray.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Transform.h"
#include "Mesh/IMesh.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class RenderContext;

	}

	namespace world
	{

class IWorldCulling;
class IWorldRenderPass;

	}

	namespace mesh
	{

class IMeshParameterCallback;
class StaticMesh;

/*! \brief Level-of-detail mesh.
 */
class T_DLLCLASS AutoLodMesh : public IMesh
{
	T_RTTI_CLASS;

public:
	AutoLodMesh();

	const Aabb3& getBoundingBox(float lodDistance) const;

	bool supportTechnique(float lodDistance, render::handle_t technique) const;

	void render(
		float lodDistance,
		render::RenderContext* renderContext,
		const world::IWorldRenderPass& worldRenderPass,
		const Transform& lastWorldTransform,
		const Transform& worldTransform,
		float distance,
		const IMeshParameterCallback* parameterCallback
	);

private:
	friend class AutoLodMeshResource;

	float m_maxDistance;
	float m_cullDistance;
	Aabb3 m_boundingBox;
	RefArray< StaticMesh > m_lods;

	StaticMesh* getStaticMesh(float lodDistance) const;
};

	}
}

#endif	// traktor_mesh_AutoLodMesh_H
