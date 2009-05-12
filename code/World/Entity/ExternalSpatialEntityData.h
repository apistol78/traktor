#ifndef traktor_world_ExternalSpatialEntityData_H
#define traktor_world_ExternalSpatialEntityData_H

#include "Core/Heap/Ref.h"
#include "World/Entity/SpatialEntityData.h"
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

/*! \brief External spatial entity data.
 * \ingroup World
 */
class T_DLLCLASS ExternalSpatialEntityData : public SpatialEntityData
{
	T_RTTI_CLASS(ExternalSpatialEntityData)

public:
	ExternalSpatialEntityData();

	ExternalSpatialEntityData(const Guid& guid);

	void setGuid(const Guid& guid);

	const Guid& getGuid() const;

	virtual bool serialize(Serializer& s);

private:
	Guid m_guid;
};

	}
}

#endif	// traktor_world_ExternalSpatialEntityData_H
