#ifndef traktor_spray_MeshRenderer_H
#define traktor_spray_MeshRenderer_H

#include <map>
#include "Core/Object.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Spray/Point.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class InstanceMesh;

	}

	namespace render
	{

class RenderContext;

	}

	namespace world
	{

class IWorldRenderPass;

	}

	namespace spray
	{

/*! \brief Mesh particle renderer.
 * \ingroup Spray
 */
class T_DLLCLASS MeshRenderer : public Object
{
	T_RTTI_CLASS;

public:
	MeshRenderer();

	virtual ~MeshRenderer();

	void destroy();

	void render(
		mesh::InstanceMesh* mesh,
		bool meshOrientationFromVelocity,
		const PointVector& points
	);

	void flush(
		render::RenderContext* renderContext,
		world::IWorldRenderPass& worldRenderPass
	);

private:
	std::map< Ref< mesh::InstanceMesh >, std::pair< PointVector, bool > > m_meshes;
	AlignedVector< mesh::InstanceMesh::instance_distance_t > m_instances;
};

	}
}

#endif	// traktor_spray_MeshRenderer_H
