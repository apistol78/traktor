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
		std::wstring name;
		Guid material;
		bool opaque;

		Part();

		bool serialize(ISerializer& s);
	};

	virtual Ref< IMesh > createMesh(
		IStream* dataStream,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::MeshFactory* meshFactory
	) const;

	virtual bool serialize(ISerializer& s);

private:
	friend class StreamMeshConverter;

	std::vector< uint32_t > m_frameOffsets;
	Aabb m_boundingBox;
	std::vector< Part > m_parts;
};

	}
}

#endif	// traktor_mesh_StreamMeshResource_H
