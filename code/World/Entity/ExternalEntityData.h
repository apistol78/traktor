#ifndef traktor_world_ExternalEntityData_H
#define traktor_world_ExternalEntityData_H

#include "Core/Guid.h"
#include "World/Entity/EntityData.h"

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

/*! \brief External entity data.
 * \ingroup World
 */
class T_DLLCLASS ExternalEntityData : public EntityData
{
	T_RTTI_CLASS;

public:
	ExternalEntityData();

	ExternalEntityData(const Guid& guid);

	void setGuid(const Guid& guid);

	const Guid& getGuid() const;

	virtual bool serialize(ISerializer& s);

private:
	Guid m_guid;
};

	}
}

#endif	// traktor_world_ExternalEntityData_H
