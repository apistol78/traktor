#include "Core/Math/TransformPath.h"
#include "Shape/Editor/Spline/CloneLayer.h"
#include "World/Entity.h"
#include "World/IEntityBuilder.h"
#include "World/WorldBuildContext.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.CloneLayer", CloneLayer, SplineLayerComponent)

CloneLayer::CloneLayer(
	const world::IEntityBuilder* builder,
	world::EntityData* entity,
	float distance
)
:	m_builder(builder)
,	m_entity(entity)
,	m_distance(distance)
{
}

void CloneLayer::destroy()
{
	for (auto entity : m_entities)
		entity->destroy();

	m_entities.clear();
}

void CloneLayer::setOwner(world::ComponentEntity* owner)
{
}

void CloneLayer::setTransform(const Transform& transform)
{
}

Aabb3 CloneLayer::getBoundingBox() const
{
	return Aabb3();
}

void CloneLayer::update(const world::UpdateParams& update)
{
	for (auto entity : m_entities)
		entity->update(update);
}

void CloneLayer::pathChanged(const TransformPath& path)
{
	m_entities.clear();

	Vector4 last = path.evaluate(0.0f).position;
	float until = 0.0f;

	int32_t steps = (int32_t)(path.getKeys().size() * 100);
	for (int32_t i = 0; i < steps; ++i)
	{
		float at = (float)i / (steps - 1);

		Matrix44 T = path.evaluate(at).transform().toMatrix44();

		Vector4 current = T.translation();
		float travel = (current - last).xyz0().length();

		if ((until -= travel) <= 0.0f)
		{
			if (true) // (m_automaticOrientation)
			{
				Transform Tn = path.evaluate(at + 0.001f).transform();
				T = lookAt(T.translation().xyz1(), Tn.translation().xyz1()).inverse();
			}

			Ref< world::Entity > entity = m_builder->create(m_entity);
			if (entity)
			{
				entity->setTransform(Transform(T));
				m_entities.push_back(entity);
			}

			until = m_distance + until;
		}

		last = current;
	}
}

void CloneLayer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	for (auto entity : m_entities)
		context.build(worldRenderView, worldRenderPass, entity);
}

	}
}
