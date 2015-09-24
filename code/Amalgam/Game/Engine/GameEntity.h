#ifndef traktor_amalgam_GameEntity_H
#define traktor_amalgam_GameEntity_H

#include "Core/RefArray.h"
#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IEntityComponent;

	}

	namespace amalgam
	{

/*! \brief Simple game entity.
 * \ingroup Amalgam
 */
class T_DLLCLASS GameEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	GameEntity();

	virtual void destroy();

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb3 getBoundingBox() const;

	virtual void update(const world::UpdateParams& update);

	/*! \brief Set controlled entity.
	 *
	 * The controlled entity is usually the visual representation
	 * of this game entity; thus a mesh entity or a like.
	 *
	 * \param entity Controlled entity.
	 */
	void setEntity(world::Entity* entity) { m_entity = entity; }

	/*! \brief Get controlled entity.
	 *
	 * \return Controlled entity.
	 */
	world::Entity* getEntity() const { return m_entity; }

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
	friend class GameEntityFactory;

	RefArray< world::IEntityComponent > m_components;
	Ref< world::Entity > m_entity;
	bool m_visible;
};

	}
}

#endif	// traktor_amalgam_GameEntity_H
