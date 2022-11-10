#pragma once

#include <map>
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Transform.h"
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

namespace traktor::render
{

class Buffer;
class IRenderSystem;
class ITexture;
class Mesh;
class RenderContext;

}

namespace traktor::world
{

class IWorldRenderPass;

}

namespace traktor::mesh
{

class IMeshParameterCallback;

/*! Blend mesh
 * \ingroup Mesh
 *
 * A blend mesh is mesh which is calculated
 * from several blend mesh shapes before being
 * rendered.
 */
class T_DLLCLASS BlendMesh : public IMesh
{
	T_RTTI_CLASS;

public:
	enum { VertexBufferCount = 3 };

	struct Instance : public Object
	{
		Ref< render::Buffer > vertexBuffers[VertexBufferCount];
		Ref< render::Mesh > mesh;
		AlignedVector< float > weights;
		uint32_t count = 0;
	};

	const Aabb3& getBoundingBox() const;

	bool supportTechnique(render::handle_t technique) const;

	uint32_t getBlendTargetCount() const;

	Ref< Instance > createInstance() const;

	void build(
		render::RenderContext* renderContext,
		const world::IWorldRenderPass& worldRenderPass,
		const Transform& lastWorldTransform,
		const Transform& worldTransform,
		Instance* instance,
		const AlignedVector< float >& blendWeights,
		float distance,
		const IMeshParameterCallback* parameterCallback
	);

	const std::map< std::wstring, int >& getBlendTargetMap() const;

private:
	friend class BlendMeshResource;

	struct Part
	{
		render::handle_t shaderTechnique;
		uint32_t meshPart;
	};

	Ref< render::IRenderSystem > m_renderSystem;
	resource::Proxy< render::Shader > m_shader;
	RefArray< render::Mesh > m_meshes;
	AlignedVector< const uint8_t* > m_vertices;
	SmallMap< render::handle_t, AlignedVector< Part > > m_parts;
	std::map< std::wstring, int > m_targetMap;
};

}
