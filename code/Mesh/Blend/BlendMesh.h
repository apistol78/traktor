#ifndef traktor_mesh_BlendMesh_H
#define traktor_mesh_BlendMesh_H

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

class RenderSystem;
class RenderContext;
class Mesh;
class Texture;

	}

	namespace world
	{

class WorldRenderView;

	}

	namespace mesh
	{

class IMeshParameterCallback;

/*! \brief Blend mesh.
 *
 * A blend mesh is mesh which is calculated
 * from several blend mesh shapes before being
 * rendered.
 */
class T_DLLCLASS BlendMesh : public Object
{
	T_RTTI_CLASS(BlendMesh)

public:
	struct Instance : public Object
	{
		Ref< render::Mesh > mesh;
	};

	BlendMesh();

	const Aabb& getBoundingBox() const;

	uint32_t getBlendTargetCount() const;

	Instance* createInstance() const;

	void render(
		render::RenderContext* renderContext,
		const world::WorldRenderView* worldRenderView,
		const Matrix44& worldTransform,
		Instance* instance,
		const std::vector< float >& blendWeights,
		float distance,
		const IMeshParameterCallback* parameterCallback
	);

	const std::map< std::wstring, int >& getBlendTargetMap() const;

private:
	friend class BlendMeshFactory;

	struct Part
	{
		resource::Proxy< render::Shader > material;
	};

	Ref< render::RenderSystem > m_renderSystem;
	RefArray< render::Mesh > m_meshes;
	std::vector< const uint8_t* > m_vertices;
	std::vector< Part > m_parts;
	std::map< std::wstring, int > m_targetMap;
};

	}
}

#endif	// traktor_mesh_BlendMesh_H
