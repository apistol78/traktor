#ifndef traktor_mesh_PartitionMesh_H
#define traktor_mesh_PartitionMesh_H

#include "Core/Math/Aabb.h"
#include "Core/Math/Matrix44.h"
#include "Mesh/IMesh.h"
#include "Render/Shader.h"
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
	namespace render
	{

class RenderContext;
class Mesh;
class ITexture;

	}

	namespace world
	{

class WorldRenderView;

	}

	namespace mesh
	{

class IMeshParameterCallback;
class IPartition;

/*! \brief Partition mesh.
 */
class T_DLLCLASS PartitionMesh : public IMesh
{
	T_RTTI_CLASS;

public:
	struct Part
	{
		render::handle_t shaderTechnique;
		uint32_t meshPart;
		bool opaque;
	};

	PartitionMesh();

	const Aabb& getBoundingBox() const;
	
	void render(
		render::RenderContext* renderContext,
		const world::WorldRenderView* worldRenderView,
		const Transform& worldTransform,
		const Transform& worldTransformPrevious,
		float distance,
		float userParameter,
		const IMeshParameterCallback* parameterCallback
	);

private:
	friend class PartitionMeshResource;

	resource::Proxy< render::Shader > m_shader;
	Ref< render::Mesh > m_mesh;
	std::vector< Part > m_parts;
	Ref< IPartition > m_partition;
#if defined(_DEBUG)
	std::string m_name;
#endif
};

	}
}

#endif	// traktor_mesh_PartitionMesh_H
