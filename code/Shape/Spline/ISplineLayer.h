#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IWorldRenderPass;
struct UpdateParams;
class WorldBuildContext;
class WorldRenderView;

	}

	namespace shape
	{

class T_DLLCLASS ISplineLayer : public Object
{
	T_RTTI_CLASS;

public:
	virtual void pathChanged() = 0;

	virtual void update(const world::UpdateParams& update) = 0;

	virtual void build(
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	) = 0;
};

	}
}
