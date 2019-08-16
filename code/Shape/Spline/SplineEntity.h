#pragma once

#include "Core/RefArray.h"
#include "Core/Math/TransformPath.h"
#include "World/Entity/ComponentEntity.h"

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
class WorldContext;
class WorldRenderView;

	}

	namespace shape
	{

class ControlPointEntity;
class ISplineLayer;

/*! Spline entity.
 * \ingroup Shape
 */
class T_DLLCLASS SplineEntity : public world::ComponentEntity
{
	T_RTTI_CLASS;

public:
	explicit SplineEntity(const Transform& transform);

	void addControlPointEntity(ControlPointEntity* controlPointEntity);

	void addLayer(ISplineLayer* layer);

	virtual void setTransform(const Transform& transform) override;

	virtual void update(const world::UpdateParams& update) override;

	void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	);

	const TransformPath& getPath() const { return m_path; }

private:
	RefArray< ControlPointEntity > m_controlPointEntities;
	RefArray< ISplineLayer > m_layers;
	TransformPath m_path;
	bool m_dirty;
};

	}
}

