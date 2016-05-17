#ifndef traktor_mesh_MeshEntityPipeline_H
#define traktor_mesh_MeshEntityPipeline_H

#include "World/Editor/EntityPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class T_DLLCLASS MeshEntityPipeline : public world::EntityPipeline
{
	T_RTTI_CLASS;

public:
	virtual TypeInfoSet getAssetTypes() const T_OVERRIDE T_FINAL;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_mesh_MeshEntityPipeline_H
