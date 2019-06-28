#pragma once

#include "Core/RefArray.h"
#include "World/EntityData.h"

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

class IEntityComponentData;

/*! Entity with components.
 * \ingroup World
 */
class T_DLLCLASS ComponentEntityData : public EntityData
{
	T_RTTI_CLASS;

public:
	/*! Set component.
	 *
	 * Only one component instance of each type is supported,
	 * thus existing instance will be replaced.
	 *
	 * \param component Component instance to set.
	 */
	void setComponent(IEntityComponentData* component);

	/*! Remove component.
	 */
	void removeComponent(IEntityComponentData* component);

	/*! Get component of type.
	 */
	IEntityComponentData* getComponent(const TypeInfo& componentType) const;

	/*! Get component of type.
	 */
	template < typename ComponentDataType >
	ComponentDataType* getComponent() const
	{
		return checked_type_cast< ComponentDataType* >(getComponent(type_of< ComponentDataType >()));
	}

	/*! Get components.
	 */
	const RefArray< IEntityComponentData >& getComponents() const;

	virtual void serialize(ISerializer& s) override;

private:
	friend class ComponentEntityPipeline;
	friend class WorldEntityFactory;

	RefArray< IEntityComponentData > m_components;
};

	}
}
