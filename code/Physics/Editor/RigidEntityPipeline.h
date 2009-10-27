#ifndef traktor_physics_RigidEntityPipeline_H
#define traktor_physics_RigidEntityPipeline_H

#include "World/Editor/EntityPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class T_DLLCLASS RigidEntityPipeline : public world::EntityPipeline
{
	T_RTTI_CLASS(RigidEntityPipeline)

public:
	virtual TypeSet getAssetTypes() const;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const Serializable* sourceAsset,
		Ref< const Object >& outBuildParams
	) const;
};

	}
}

#endif	// traktor_physics_RigidEntityPipeline_H
