#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Shape/Spline/ISplineLayer.h"

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

class Entity;
class EntityData;
class IEntityBuilder;

	}

	namespace shape
	{

class SplineEntity;

/*! \brief
 * \ingroup Shape
 */
class T_DLLCLASS CloneLayer : public ISplineLayer
{
	T_RTTI_CLASS;

public:
	CloneLayer(
		SplineEntity* owner,
		const world::IEntityBuilder* builder,
		world::EntityData* entity,
		float distance
	);

	virtual void update(const world::UpdateParams& update) override final;

	virtual void pathChanged() override final;

	virtual void build(
		const world::WorldContext& worldContext,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	) override final;

private:
	SplineEntity* m_owner;
	Ref< const world::IEntityBuilder > m_builder;
	Ref< world::EntityData > m_entity;
	float m_distance;

	RefArray< world::Entity > m_entities;
};

	}
}
