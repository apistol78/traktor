#ifndef traktor_amalgam_GameEntity_H
#define traktor_amalgam_GameEntity_H

#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
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
		world::IEntityEventManager* eventManager
	);

	virtual void destroy();

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb3 getBoundingBox() const;

	virtual void update(const world::UpdateParams& update);

	world::IEntityEventInstance* raiseEvent(const std::wstring& eventId);

	world::IEntityEventInstance* raiseEvent(const std::wstring& eventId, const Transform& Toffset);

	void setTag(const std::wstring& tag) { m_tag = tag; }

	const std::wstring& getTag() const { return m_tag; }

	void setObject(Object* object) { m_object = object; }

	Object* getObject() const { return m_object; }

	void setEntity(world::Entity* entity) { m_entity = entity; }

	world::Entity* getEntity() const { return m_entity; }

private:
	std::wstring m_tag;
	Ref< Object > m_object;
	Ref< world::Entity > m_entity;
	Ref< world::EntityEventSet > m_eventSet;
	Ref< world::IEntityEventManager > m_eventManager;
};

	}
}

#endif	// traktor_amalgam_GameEntity_H
