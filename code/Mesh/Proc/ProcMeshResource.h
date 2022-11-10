#pragma once

#include "Mesh/MeshResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

class T_DLLCLASS ProcMeshResource : public MeshResource
{
	T_RTTI_CLASS;

public:
	virtual Ref< IMesh > createMesh(
		const std::wstring& name,
		IStream* dataStream,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::MeshFactory* meshFactory
	) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class ProcMeshConverter;
};

}
