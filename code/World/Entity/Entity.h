#ifndef traktor_world_Entity_H
#define traktor_world_Entity_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class EntityUpdate;

/*! \brief World entity base class.
 * \ingroup World
 *
 * All accessible entities must be derived from this class.
 * Not all entities must be renderable, it's also
 * designed to be able to handle non-renderables such as triggers etc.
 */
class T_DLLCLASS Entity : public Object
{
	T_RTTI_CLASS(Entity)

public:
	virtual ~Entity();

	/*! \brief Destroy entity resources.
	 *
	 * Called automatically from Entity destructor
	 * in order to destroy any resources allocated
	 * by the entity.
	 *
	 * \note This may be called multiple times for
	 * a single entity so care must be taken when
	 * performing the destruction.
	 */
	virtual void destroy();

	/*! \brief Update entity state.
	 *
	 * Should be called frequently in order to update
	 * each entity's state.
	 *
	 * \param update Update message.
	 */
	virtual void update(const EntityUpdate* update) = 0;

	/*! \brief Add entity reference.
	 *
	 * \note This method doesn't do anything,
	 * if you want to keep references then you
	 * have to overload this method.
	 *
	 * \param name Name of reference.
	 * \param reference Entity reference.
	 */
	virtual void addReference(const std::wstring& name, Entity* reference);

	/*! \brief Add entity referee.
	 *
	 * \note This method doesn't do anything,
	 * if you want to keep references then you
	 * have to overload this method.
	 *
	 * \param name Name of referee.
	 * \param reference Entity referee.
	 */
	virtual void addReferee(const std::wstring& name, Entity* referee);
};

	}
}

#endif	// traktor_world_Entity_H
