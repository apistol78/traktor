/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_SkinnedMesh_H
#define traktor_mesh_SkinnedMesh_H

#include <map>
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Aabb3.h"
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

	const Aabb3& getBoundingBox() const;

	bool supportTechnique(render::handle_t technique) const;

	void render(
		render::RenderContext* renderContext,
		world::IWorldRenderPass& worldRenderPass,
		const Transform& lastWorldTransform,
		const Transform& worldTransform,
		const AlignedVector< Vector4 >& jointTransforms,
		float distance,
		const IMeshParameterCallback* parameterCallback
	);

	int32_t getJointCount() const;

	const std::map< std::wstring, int32_t >& getJointMap() const;

private:
	friend class SkinnedMeshResource;

	struct Part
	{
		render::handle_t shaderTechnique;
		uint32_t meshPart;
	};

	resource::Proxy< render::Shader > m_shader;
	Ref< render::Mesh > m_mesh;
	SmallMap< render::handle_t, std::vector< Part > > m_parts;
	std::map< std::wstring, int32_t > m_jointMap;
	int32_t m_jointCount;
#if defined(_DEBUG)
	std::string m_name;
#endif
};

	}
}

#endif	// traktor_mesh_SkinnedMesh_H
