#ifndef traktor_world_SwitchEntity_H
#define traktor_world_SwitchEntity_H

#include "Core/RefArray.h"
#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Switch of entities.
 * \ingroup World
 */
class T_DLLCLASS SwitchEntity : public Entity
{
	T_RTTI_CLASS;

public:
	SwitchEntity();

	SwitchEntity(const Transform& transform, int32_t active);

	virtual void destroy();

	int32_t addEntity(Entity* entity);

	Entity* getEntity(int32_t index) const;

	Entity* setActive(int32_t active);

	int32_t getActive() const;

	Entity* getActiveEntity() const;
	
	virtual void update(const UpdateParams& update) T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual bool getTransform(Transform& outTransform) const T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;
	
private:
	Transform m_transform;
	RefArray< Entity > m_entities;
	int32_t m_active;
};
	
	}
}

#endif	// traktor_world_SwitchEntity_H
