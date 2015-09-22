#ifndef traktor_amalgam_GameEntity_H
#define traktor_amalgam_GameEntity_H

#include "Resource/Proxy.h"
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

class IRuntimeClass;

	namespace world
	{

class EntityEventSet;
class IEntityEventInstance;
class IEntityEventManager;

	}

	namespace amalgam
	{

/*! \brief Simple game entity.
 * \ingroup Amalgam
 *
 * The "simple game entity" is designed to be accessed and controlled
 * from scripts. As it's a general, feature-rich, game entity it's also
 * quite heavy memory wise, thus if many entities are required
 * please consider writing custom entities instead.
 *
 * \note
 * As this is more or less a really simplified game entity
 * it's probably only suitable for smaller games.
 */
class T_DLLCLASS GameEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	GameEntity(
		const std::wstring& tag,
		Object* object,
		world::Entity* entity,
		world::EntityEventSet* eventSet,
		world::IEntityEventManager* eventManager,
		const resource::Proxy< IRuntimeClass >& clazz
	);

	virtual void destroy();

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb3 getBoundingBox() const;

	virtual void update(const world::UpdateParams& update);

	/*! \brief Raise attached event.
	 *
	 * \param eventId Name of event.
	 * \return Instance of raised event.
	 */
	world::IEntityEventInstance* raiseEvent(const std::wstring& eventId);

	/*! \brief Raise attached event.
	 *
	 * \param eventId Name of event.
	 * \param Toffset Transformation offset from this entity.
	 * \return Instance of raised event.
	 */
	world::IEntityEventInstance* raiseEvent(const std::wstring& eventId, const Transform& Toffset);

	/*! \brief Set string tag.
	 *
	 * \param tag String tag.
	 */
	void setTag(const std::wstring& tag) { m_tag = tag; }

	/*! \brief Get string tag.
	 *
	 * \return String tag.
	 */
	const std::wstring& getTag() const { return m_tag; }

	/*! \brief Set user-defined object.
	 *
	 * \param object User-defined object.
	 */
	void setObject(Object* object) { m_object = object; }

	/*! \brief Get user-defined object.
	 *
	 * \return User-defined object.
	 */
	Object* getObject() const { return m_object; }

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
	std::wstring m_tag;
	Ref< Object > m_object;
	Ref< world::Entity > m_entity;
	Ref< world::EntityEventSet > m_eventSet;
	Ref< world::IEntityEventManager > m_eventManager;
	resource::Proxy< IRuntimeClass > m_class;
	Ref< ITypedObject > m_classObject;
	bool m_visible;
};

	}
}

#endif	// traktor_amalgam_GameEntity_H
