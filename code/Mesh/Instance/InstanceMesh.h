#ifndef traktor_mesh_InstanceMesh_H
#define traktor_mesh_InstanceMesh_H

#include <map>
#include "Core/Math/Aabb.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector4.h"
#include "Core/Containers/AlignedVector.h"
#include "Mesh/IMesh.h"
#include "Mesh/Instance/InstanceMeshData.h"
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
class T_DLLCLASS InstanceMesh : public IMesh
{
	T_RTTI_CLASS;

public:
#if TARGET_OS_IPHONE
	enum { MaxInstanceCount = 10 };		// ES doesn't support 32-bit indices thus we cannot batch enough instances.
#else
	enum { MaxInstanceCount = 20 };
#endif

	struct Part
	{
		resource::Proxy< render::Shader > material;
		bool opaque;
	};

	typedef std::pair< InstanceMeshData, float > instance_distance_t;

	InstanceMesh();

	virtual ~InstanceMesh();

	const Aabb& getBoundingBox() const;
	
	void render(render::RenderContext* renderContext, const world::WorldRenderView* worldRenderView, AlignedVector< instance_distance_t >& instanceWorld);

	inline const Ref< render::Mesh >& getRenderMesh() const { return m_mesh; }

	inline const std::vector< Part >& getParts() const { return m_parts; }

private:
	friend class InstanceMeshResource;

	Ref< render::Mesh > m_mesh;
	std::vector< Part > m_parts;

#if defined(_DEBUG)
	uint32_t m_instanceUsedCount;
	uint32_t m_instanceRenderedCount;
#endif
};

	}
}

#endif	// traktor_mesh_InstanceMesh_H
