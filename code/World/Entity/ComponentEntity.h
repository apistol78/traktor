#ifndef traktor_world_ComponentEntity_H
#define traktor_world_ComponentEntity_H

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

class IEntityComponent;

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS ComponentEntity : public Entity
{
	T_RTTI_CLASS;

public:
	ComponentEntity();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual bool getTransform(Transform& outTransform) const T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void update(const UpdateParams& update) T_OVERRIDE T_FINAL;

	/*! \brief Set controlled entity.
	 *
	 * The controlled entity is usually the visual representation
	 * of this game entity; thus a mesh entity or a like.
	 *
	 * \param entity Controlled entity.
	 */
	void setEntity(Entity* entity) { m_entity = entity; }

	/*! \brief Get controlled entity.
	 *
	 * \return Controlled entity.
	 */
	Entity* getEntity() const { return m_entity; }

	/*! \brief Determine if controlled entity should be visible.
	 *
	 * \param visible True if controlled entity should be visible.
	 */
	void setVisible(bool visible) { m_visible = visible; }

	/*! \brief Return true if controlled entity is visible.
	 *
	 * \return True if controlled entity is visible.
	 */
	bool isVisible() const { return m_visible; }

private:
	friend class WorldEntityFactory;

	RefArray< IEntityComponent > m_components;
	Ref< Entity > m_entity;
	bool m_visible;
};

	}
}

#endif	// traktor_world_ComponentEntity_H
