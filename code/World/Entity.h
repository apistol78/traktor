#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Transform.h"
#include "World/WorldTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class IEntityComponent;

/*! World entity class.
 * \ingroup World
 */
class T_DLLCLASS Entity : public Object
{
	T_RTTI_CLASS;

public:
	Entity() = default;

	Entity(const Entity&) = delete;

	explicit Entity(const std::wstring& name, const Transform& transform);

	explicit Entity(const std::wstring& name, const Transform& transform, const RefArray< IEntityComponent >& components);

	virtual ~Entity();

	/*! Destroy entity resources.
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

	/*! Get entity name. */
	const std::wstring& getName() const;

	/*! Set entity transform.
	 *
	 * \param transform Entity transform.
	 */
	virtual void setTransform(const Transform& transform);

	/*! Get entity transform.
	 *
	 * \return Entity transform.
	 */
	virtual Transform getTransform() const;

	/*! Get entity bounding box.
	 * Return entity bounding box in entity space.
	 *
	 * \return Entity bounding box.
	 */
	virtual Aabb3 getBoundingBox() const;

	/*! Update entity.
	 *
	 * \param update Update parameters.
	 */
	virtual void update(const UpdateParams& update);

	/*! Set component in entity.
	 *
	 * \param component Component instance.
	 */
	void setComponent(IEntityComponent* component);

	/*! Get component of type.
	 *
	 * \param componentType Type of component.
	 * \return Component instance matching type.
	 */
	IEntityComponent* getComponent(const TypeInfo& componentType) const;

	/*! Get component of type.
	 *
	 * \param ComponentType Type of component.
	 * \return Component instance matching type.
	 */
	template < typename ComponentType >
	ComponentType* getComponent() const
	{
		return checked_type_cast< ComponentType* >(getComponent(type_of< ComponentType >()));
	}

	/*! Get components.
	 *
	 * \return Array of all components.
	 */
	const RefArray< IEntityComponent >& getComponents() const
	{
		return m_components;
	}

private:
	std::wstring m_name;
	Transform m_transform = Transform::identity();
	RefArray< IEntityComponent > m_components;
	const IEntityComponent* m_updating = nullptr;
};

}
