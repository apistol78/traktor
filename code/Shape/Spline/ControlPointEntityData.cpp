#include "Shape/Spline/ControlPointEntity.h"
#include "Shape/Spline/ControlPointEntityData.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.ControlPointEntityData", 0, ControlPointEntityData, world::ComponentEntityData)

Ref< ControlPointEntity > ControlPointEntityData::createEntity(const world::IEntityBuilder* builder) const
{
	Ref< ControlPointEntity > entity = new ControlPointEntity(getTransform());

	for (auto cd : getComponents())
	{
		Ref< world::IEntityComponent > component = builder->create(cd);
		if (!component)
			continue;
		entity->setComponent(component);
	}

	return entity;
}

void ControlPointEntityData::serialize(ISerializer& s)
{
	world::ComponentEntityData::serialize(s);
}

	}
}
