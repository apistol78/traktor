#ifndef traktor_world_EntityData_H
#define traktor_world_EntityData_H

#include <string>
#include "Core/Serialization/Serializable.h"

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

/*! \brief Entity data.
 * \ingroup World
 *
 * Serialized data used to create runtime entities.
 * The entity data is stored separately in order to
 * save runtime memory as the data structure is
 * normally tossed away when the entity has been created.
 */
class T_DLLCLASS EntityData : public Serializable
{
	T_RTTI_CLASS(EntityData)

public:
	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	virtual bool serialize(Serializer& s);

private:
	std::wstring m_name;
};

	}
}

#endif	// traktor_world_EntityData_H
