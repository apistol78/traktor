#ifndef traktor_mesh_StaticMesh_H
#define traktor_mesh_StaticMesh_H

#include "Resource/Proxy.h"
#include "Core/Object.h"
#include "Core/Math/Aabb.h"
#include "Core/Math/Matrix44.h"
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

/*! \brief Static mesh.
 *
 * A static mesh is a basic rigid mesh which at
 * all times should be placed on the GPU for
 * quick rendering.
 */
class T_DLLCLASS StaticMesh : public Object
{
	T_RTTI_CLASS(StaticMesh)

public:
	struct Part
	{
		resource::Proxy< render::Shader > material;
	};

	StaticMesh();

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

	inline const Ref< render::Mesh >& getRenderMesh() const { return m_mesh; }

	inline const std::vector< Part >& getParts() const { return m_parts; }

private:
	friend class StaticMeshFactory;

	Ref< render::Mesh > m_mesh;
	std::vector< Part > m_parts;
};

	}
}

#endif	// traktor_mesh_StaticMesh_H
