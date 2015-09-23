#ifndef traktor_sprite_SpriteEntity_H
#define traktor_sprite_SpriteEntity_H

#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRITE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sprite
	{

/*! \brief
 * \ingroup Sprite
 */
class T_DLLCLASS SpriteEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb3 getBoundingBox() const;

	virtual void update(const world::UpdateParams& update);
};

	}
}

#endif	// traktor_sprite_SpriteEntity_H
