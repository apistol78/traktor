/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_IndoorMesh_H
#define traktor_mesh_IndoorMesh_H

#include "Core/Containers/SmallMap.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Winding3.h"
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

	const Aabb3& getBoundingBox() const;

	bool supportTechnique(render::handle_t technique) const;

	void render(
		render::RenderContext* renderContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
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
	};

	struct Sector
	{
		Aabb3 boundingBox;
		SmallMap< render::handle_t, std::vector< Part > > parts;
	};

	struct Portal
	{
		Winding3 winding;
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
