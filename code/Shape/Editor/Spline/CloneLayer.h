#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Shape/Editor/Spline/SplineLayerComponent.h"

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
class EntityData;
class IEntityBuilder;
class IWorldRenderPass;
class WorldBuildContext;
class WorldRenderView;

	}

	namespace shape
	{

/*!
 * \ingroup Shape
 */
class T_DLLCLASS CloneLayer : public SplineLayerComponent
{
	T_RTTI_CLASS;

public:
	CloneLayer(
		const world::IEntityBuilder* builder,
		world::EntityData* entity,
		float distance
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	virtual void pathChanged(const TransformPath& path) override final;

	void build(
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	);

private:
	Ref< const world::IEntityBuilder > m_builder;
	Ref< world::EntityData > m_entity;
	float m_distance;

	RefArray< world::Entity > m_entities;
};

	}
}
