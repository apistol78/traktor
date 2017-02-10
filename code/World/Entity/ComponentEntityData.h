#ifndef traktor_world_ComponentEntityData_H
#define traktor_world_ComponentEntityData_H

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

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS ComponentEntityData : public EntityData
{
	T_RTTI_CLASS;

public:
	/*! \brief Set component.
	 */
	void setComponent(IEntityComponentData* component);

	/*! \brief Get component of type.
	 */
	IEntityComponentData* getComponent(const TypeInfo& componentType) const;

	/*! \brief Get component of type.
	 */
	template < typename ComponentDataType >
	ComponentDataType* getComponent() const
	{
		return checked_type_cast< ComponentDataType* >(getComponent(type_of< ComponentDataType >()));
	}

	/*! \brief Get components.
	 */
	const RefArray< IEntityComponentData >& getComponents() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class ComponentEntityPipeline;
	friend class WorldEntityFactory;

	RefArray< IEntityComponentData > m_components;
};

	}
}

#endif	// traktor_world_ComponentEntityData_H
