#ifndef traktor_mesh_InstanceMesh_H
#define traktor_mesh_InstanceMesh_H

#include <map>
#include "Resource/Proxy.h"
#include "Core/Object.h"
#include "Core/Math/Aabb.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector4.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/Shader.h"
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
	namespace render
	{

class RenderContext;
class Mesh;
class Shader;

	}

	namespace world
	{

class WorldRenderView;

	}

	namespace mesh
	{

/*! \brief Instance mesh.
 *
 * Instance meshes are meshes which are repeated
 * automatically by the GPU in any number of instances
 * using hardware instancing in a single draw call.
 */
class T_DLLCLASS InstanceMesh : public Object
{
	T_RTTI_CLASS(InstanceMesh)

public:
	enum { MaxInstanceCount = 60 };

	struct Part
	{
		resource::Proxy< render::Shader > material;
	};

	typedef std::pair< InstanceMeshData, float > instance_distance_t;

	InstanceMesh();

	const Aabb& getBoundingBox() const;
	
	void render(render::RenderContext* renderContext, const world::WorldRenderView* worldRenderView, AlignedVector< instance_distance_t >& instanceWorld);

	inline const Ref< render::Mesh >& getRenderMesh() const { return m_mesh; }

	inline const std::vector< Part >& getParts() const { return m_parts; }

private:
	friend class InstanceMeshFactory;

	Ref< render::Mesh > m_mesh;
	std::vector< Part > m_parts;
};

	}
}

#endif	// traktor_mesh_InstanceMesh_H
