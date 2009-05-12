#ifndef traktor_world_ExternalEntityData_H
#define traktor_world_ExternalEntityData_H

#include "Core/Heap/Ref.h"
#include "World/Entity/EntityData.h"
#include "Core/Guid.h"

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

/*! \brief External entity data.
 * \ingroup World
 */
class T_DLLCLASS ExternalEntityData : public EntityData
{
	T_RTTI_CLASS(ExternalEntityData)

public:
	ExternalEntityData();

	ExternalEntityData(const Guid& guid);

	void setGuid(const Guid& guid);

	const Guid& getGuid() const;

	virtual bool serialize(Serializer& s);

private:
	Guid m_guid;
};

	}
}

#endif	// traktor_world_ExternalEntityData_H
