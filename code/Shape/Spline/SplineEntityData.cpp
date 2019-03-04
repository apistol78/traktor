#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Shape/Spline/ControlPointEntity.h"
#include "Shape/Spline/ControlPointEntityData.h"
#include "Shape/Spline/ISplineLayerData.h"
#include "Shape/Spline/SplineEntity.h"
#include "Shape/Spline/SplineEntityData.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.SplineEntityData", 0, SplineEntityData, world::ComponentEntityData)

Ref< SplineEntity > SplineEntityData::createEntity(const world::IEntityBuilder* builder, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	Ref< SplineEntity > entity = new SplineEntity(getTransform());

	for (const auto data : m_controlPointEntities)
	{
		Ref< ControlPointEntity > controlPoint = builder->create< ControlPointEntity >(data);
		if (!controlPoint)
			return nullptr;

		entity->addControlPointEntity(controlPoint);
	}

	for (const auto data : m_layers)
	{
		Ref< ISplineLayer > layer = data->createLayer(entity, builder, resourceManager, renderSystem);
		if (!layer)
			return nullptr;

		entity->addLayer(layer);
	}

	for (const auto data : getComponents())
	{
		Ref< world::IEntityComponent > component = builder->create(data);
		if (!component)
			return nullptr;

		entity->setComponent(component);
	}

	return entity;
}

void SplineEntityData::addEntityData(ControlPointEntityData* controlPointEntity)
{
	T_ASSERT(controlPointEntity != nullptr);
	m_controlPointEntities.push_back(controlPointEntity);
}

void SplineEntityData::setTransform(const Transform& transform)
{
	Transform deltaTransform = transform * getTransform().inverse();
	for (auto controlPointEntity : m_controlPointEntities)
	{
		if (controlPointEntity != nullptr)
		{
			Transform currentTransform = controlPointEntity->getTransform();
			controlPointEntity->setTransform(deltaTransform * currentTransform);
		}
	}
	world::ComponentEntityData::setTransform(transform);
}

void SplineEntityData::serialize(ISerializer& s)
{
	world::ComponentEntityData::serialize(s);

	s >> MemberRefArray< ControlPointEntityData >(L"controlPointEntities", m_controlPointEntities);
	s >> MemberRefArray< ISplineLayerData >(L"layers", m_layers);
}

	}
}
