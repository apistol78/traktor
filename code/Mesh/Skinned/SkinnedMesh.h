#ifndef traktor_mesh_SkinnedMesh_H
#define traktor_mesh_SkinnedMesh_H

#include "Resource/Proxy.h"
#include "Core/Object.h"
#include "Core/Math/Aabb.h"
#include "Core/Math/Matrix44.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/Shader.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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

/*! \brief Skinned mesh.
 *
 * For each vertex the skinned mesh blends
 * the final world transform from a palette of
 * transforms using per-vertex weights.
 */
class T_DLLCLASS SkinnedMesh : public Object
{
	T_RTTI_CLASS;

public:
	SkinnedMesh();

	const Aabb& getBoundingBox() const;

	void render(
		render::RenderContext* renderContext,
		const world::WorldRenderView* worldRenderView,
		const Transform& worldTransform,
		const AlignedVector< Matrix44 >& boneTransforms,
		float distance,
		const IMeshParameterCallback* parameterCallback
	);

	const std::map< std::wstring, int >& getBoneMap() const;

private:
	friend class SkinnedMeshFactory;

	struct Part
	{
		resource::Proxy< render::Shader > material;
	};

	Ref< render::Mesh > m_mesh;
	std::vector< Part > m_parts;
	std::map< std::wstring, int > m_boneMap;
};

	}
}

#endif	// traktor_mesh_SkinnedMesh_H
