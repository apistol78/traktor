#pragma once

#include "Core/RefArray.h"
#include "Core/Math/TransformPath.h"
#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;
class IWorldRenderPass;
class WorldBuildContext;
class WorldRenderView;

	}

	namespace shape
	{

/*! Spline entity.
 * \ingroup Shape
 */
class T_DLLCLASS SplineEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	SplineEntity();

	virtual void update(const world::UpdateParams& update) override final;

	void build(
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	);

	const TransformPath& getPath() const { return m_path; }

private:
	world::Entity* m_owner;
	TransformPath m_path;
	bool m_dirty;
};

	}
}

