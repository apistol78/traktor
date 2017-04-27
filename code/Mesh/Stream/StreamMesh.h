/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_StreamMesh_H
#define traktor_mesh_StreamMesh_H

#include "Core/Containers/SmallMap.h"
#include "Core/Math/Aabb3.h"
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

class IStream;

	namespace render
	{

class ITexture;
class Mesh;
class MeshReader;
class RenderContext;

	}

	namespace world
	{

class IWorldRenderPass;

	}

	namespace mesh
	{

class IMeshParameterCallback;

/*! \brief Stream mesh.
 */
class T_DLLCLASS StreamMesh : public IMesh
{
	T_RTTI_CLASS;

public:
	struct Instance : public Object
	{
		uint32_t frame;
		Ref< render::Mesh > mesh[2];
	};

	StreamMesh();

	const Aabb3& getBoundingBox() const;

	bool supportTechnique(render::handle_t technique) const;

	uint32_t getFrameCount() const;

	Ref< Instance > createInstance() const;
	
	void render(
		render::RenderContext* renderContext,
		world::IWorldRenderPass& worldRenderPass,
		const Transform& worldTransform,
		Instance* instance,
		uint32_t frame,
		float distance,
		const IMeshParameterCallback* parameterCallback
	);

private:
	friend class StreamMeshResource;

	struct Part
	{
		render::handle_t shaderTechnique;
		std::wstring meshPart;
	};

	resource::Proxy< render::Shader > m_shader;
	Ref< IStream > m_stream;
	Ref< render::MeshReader > m_meshReader;
	std::vector< uint32_t > m_frameOffsets;
	Aabb3 m_boundingBox;
	SmallMap< render::handle_t, std::vector< Part > > m_parts;
};

	}
}

#endif	// traktor_mesh_StreamMesh_H
