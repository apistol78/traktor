#ifndef traktor_mesh_PartitionMeshResource_H
#define traktor_mesh_PartitionMeshResource_H

#include "Core/Guid.h"
#include "Core/Containers/AlignedVector.h"
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

class IPartitionData;

class T_DLLCLASS PartitionMeshResource : public IMeshResource
{
	T_RTTI_CLASS;

public:
	struct T_DLLCLASS Part
	{
		std::wstring shaderTechnique;
		uint32_t meshPart;
		Aabb3 boundingBox;

		Part();

		void serialize(ISerializer& s);
	};

	virtual Ref< IMesh > createMesh(
		const std::wstring& name,
		IStream* dataStream,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::MeshFactory* meshFactory
	) const;

	virtual void serialize(ISerializer& s);

private:
	friend class PartitionMeshConverter;

	resource::Id< render::Shader > m_shader;
	AlignedVector< Part > m_parts;
	Ref< IPartitionData > m_partitionData;
};

	}
}

#endif	// traktor_mesh_PartitionMeshResource_H
