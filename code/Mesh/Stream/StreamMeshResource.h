#ifndef traktor_mesh_StreamMeshResource_H
#define traktor_mesh_StreamMeshResource_H

#include "Core/Guid.h"
#include "Core/Math/Aabb.h"
#include "Mesh/IMeshResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
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
		bool opaque;

		Part();

		bool serialize(ISerializer& s);
	};

	virtual Ref< IMesh > createMesh(
		const std::wstring& name,
		IStream* dataStream,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::MeshFactory* meshFactory
	) const;

	virtual bool serialize(ISerializer& s);

private:
	friend class StreamMeshConverter;
	typedef std::list< Part > parts_t;

	Guid m_shader;
	std::vector< uint32_t > m_frameOffsets;
	Aabb m_boundingBox;
	std::map< std::wstring, parts_t > m_parts;
};

	}
}

#endif	// traktor_mesh_StreamMeshResource_H
