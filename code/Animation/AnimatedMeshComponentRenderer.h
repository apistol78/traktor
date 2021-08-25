#pragma once

#include "Mesh/MeshComponentRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

class T_DLLCLASS AnimatedMeshComponentRenderer : public mesh::MeshComponentRenderer
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void gather(
		const world::WorldGatherContext& context,
		Object* renderable
	) override final;
};

	}
}

