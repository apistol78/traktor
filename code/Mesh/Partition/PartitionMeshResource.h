#pragma once

#include "Core/Guid.h"
#include "Core/Containers/AlignedVector.h"
#include "Mesh/MeshResource.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Shader;

}

namespace traktor::mesh
{

class IPartitionData;

class T_DLLCLASS PartitionMeshResource : public MeshResource
{
	T_RTTI_CLASS;

public:
	struct T_DLLCLASS Part
	{
		std::wstring shaderTechnique;
		uint32_t meshPart = 0;
		Aabb3 boundingBox;

		void serialize(ISerializer& s);
	};

	virtual Ref< IMesh > createMesh(
		const std::wstring& name,
		IStream* dataStream,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::MeshFactory* meshFactory
	) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class PartitionMeshConverter;

	resource::Id< render::Shader > m_shader;
	AlignedVector< Part > m_parts;
	Ref< IPartitionData > m_partitionData;
};

}
