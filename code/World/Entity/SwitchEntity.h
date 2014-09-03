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

	void setActive(int32_t active);

	int32_t getActive() const;

	Entity* getActiveEntity() const;
	
	virtual void update(const UpdateParams& update);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb3 getBoundingBox() const;
	
private:
	Transform m_transform;
	RefArray< Entity > m_entities;
	int32_t m_active;
};
	
	}
}

#endif	// traktor_world_SwitchEntity_H
