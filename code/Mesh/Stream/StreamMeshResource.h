#ifndef traktor_mesh_StreamMeshResource_H
#define traktor_mesh_StreamMeshResource_H

#include "Core/Guid.h"
#include "Core/Math/Aabb3.h"
#include "Mesh/IMeshResource.h"
#include "Resource/Id.h"

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

class Shader;

	}

	namespace mesh
	{

class T_DLLCLASS StreamMeshResource : public IMeshResource
{
	T_RTTI_CLASS;

public:
	struct T_DLLCLASS Part
	{
		std::wstring shaderTechnique;
		std::wstring meshPart;

		Part();

		void serialize(ISerializer& s);
	};

	virtual Ref< IMesh > createMesh(
		const std::wstring& name,
		IStream* dataStream,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::MeshFactory* meshFactory
	) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class StreamMeshConverter;
	typedef std::list< Part > parts_t;

	resource::Id< render::Shader > m_shader;
	std::vector< uint32_t > m_frameOffsets;
	Aabb3 m_boundingBox;
	std::map< std::wstring, parts_t > m_parts;
};

	}
}

#endif	// traktor_mesh_StreamMeshResource_H
