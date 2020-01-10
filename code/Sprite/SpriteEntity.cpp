#include "Sprite/SpriteEntity.h"

namespace traktor
{
	namespace sprite
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sprite.SpriteEntity", SpriteEntity, world::Entity)

void SpriteEntity::setTransform(const Transform& transform)
{
}

Transform SpriteEntity::getTransform() const
{
	return Transform::identity();
}

Aabb3 SpriteEntity::getBoundingBox() const
{
	return Aabb3();
}

void SpriteEntity::update(const world::UpdateParams& update)
{
}

	}
}
