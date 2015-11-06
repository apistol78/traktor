#ifndef traktor_world_VolumeComponentData_H
#define traktor_world_VolumeComponentData_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb3.h"
#include "World/IEntityComponentData.h"

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

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS VolumeComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s);

	const AlignedVector< Aabb3 >& getVolumes() const { return m_volumes; }

private:
	AlignedVector< Aabb3 > m_volumes;
};

	}
}

#endif	// traktor_world_VolumeComponentData_H
