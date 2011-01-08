#ifndef traktor_mesh_SkinnedMesh_H
#define traktor_mesh_SkinnedMesh_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb.h"
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

class IWorldRenderPass;

	}

	namespace mesh
	{

class IMeshParameterCallback;

/*! \brief Skinned mesh.
 *
 * For each vertex the skinned mesh blends
 * the final world transform from a palette of
 * transforms using per-vertex weights.
 */
class T_DLLCLASS SkinnedMesh : public IMesh
{
	T_RTTI_CLASS;

public:
	SkinnedMesh();

	const Aabb& getBoundingBox() const;

	void render(
		render::RenderContext* renderContext,
		world::IWorldRenderPass& worldRenderPass,
		const Transform& worldTransform,
		const AlignedVector< Vector4 >& boneTransforms,
		float distance,
		const IMeshParameterCallback* parameterCallback
	);

	int32_t getBoneCount() const;

	const std::map< std::wstring, int32_t >& getBoneMap() const;

private:
	friend class SkinnedMeshResource;

	struct Part
	{
		render::handle_t shaderTechnique;
		uint32_t meshPart;
		bool opaque;
	};

	resource::Proxy< render::Shader > m_shader;
	Ref< render::Mesh > m_mesh;
	std::map< render::handle_t, std::vector< Part > > m_parts;
	std::map< std::wstring, int32_t > m_boneMap;
	int32_t m_boneCount;
};

	}
}

#endif	// traktor_mesh_SkinnedMesh_H
