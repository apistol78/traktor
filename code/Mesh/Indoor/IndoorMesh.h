#ifndef traktor_mesh_IndoorMesh_H
#define traktor_mesh_IndoorMesh_H

#include "Core/Math/Aabb.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Winding.h"
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

class T_DLLCLASS IndoorMesh : public IMesh
{
	T_RTTI_CLASS;

public:
	IndoorMesh();

	const Aabb& getBoundingBox() const;

	void render(
		render::RenderContext* renderContext,
		const world::WorldRenderView* worldRenderView,
		const Transform& worldTransform,
		float distance,
		const IMeshParameterCallback* parameterCallback
	);

private:
	friend class IndoorMeshResource;

	struct Part
	{
		render::handle_t shaderTechnique;
		uint32_t meshPart;
		bool opaque;
	};

	struct Sector
	{
		Aabb boundingBox;
		std::map< render::handle_t, std::vector< Part > > parts;
	};

	struct Portal
	{
		Winding winding;
		int sectorA;
		int sectorB;
	};

	Ref< render::Mesh > m_mesh;
	resource::Proxy< render::Shader > m_shader;
	std::vector< Part > m_parts;
	AlignedVector< Sector > m_sectors;
	AlignedVector< Portal > m_portals;

	void findVisibleSectors(
		const AlignedVector< Plane >& frustum,
		const Matrix44& view,
		int currentSector,
		std::set< int >& outVisibleSectors
	);
};

	}
}

#endif	// traktor_mesh_IndoorMesh_H
