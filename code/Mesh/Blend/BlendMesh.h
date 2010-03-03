#ifndef traktor_mesh_BlendMesh_H
#define traktor_mesh_BlendMesh_H

#include "Core/RefArray.h"
#include "Core/Math/Aabb.h"
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

namespace traktor
{
	namespace render
	{

class IRenderSystem;
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

/*! \brief Blend mesh
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
	struct Instance : public Object
	{
		Ref< render::Mesh > mesh;
		std::vector< float > weights;
		bool opaque;
	};

	BlendMesh();

	const Aabb& getBoundingBox() const;

	uint32_t getBlendTargetCount() const;

	Ref< Instance > createInstance() const;

	void render(
		render::RenderContext* renderContext,
		const world::WorldRenderView* worldRenderView,
		const Transform& worldTransform,
		Instance* instance,
		const std::vector< float >& blendWeights,
		float distance,
		const IMeshParameterCallback* parameterCallback
	);

	const std::map< std::wstring, int >& getBlendTargetMap() const;

private:
	friend class BlendMeshResource;

	struct Part
	{
		resource::Proxy< render::Shader > material;
		bool opaque;
	};

	Ref< render::IRenderSystem > m_renderSystem;
	RefArray< render::Mesh > m_meshes;
	std::vector< const uint8_t* > m_vertices;
	std::vector< Part > m_parts;
	std::map< std::wstring, int > m_targetMap;
};

	}
}

#endif	// traktor_mesh_BlendMesh_H
