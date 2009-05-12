#ifndef traktor_animation_PathEntityData_H
#define traktor_animation_PathEntityData_H

#include "Core/Heap/Ref.h"
#include "World/Entity/SpatialEntityData.h"
#include "Animation/PathEntity/PathEntity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class EntityBuilder;

	}

	namespace animation
	{

/*! \brief Movement path entity data.
 * \ingroup Animation
 */
class T_DLLCLASS PathEntityData : public world::SpatialEntityData
{
	T_RTTI_CLASS(PathEntityData)

public:
	PathEntityData();

	PathEntity* createEntity(world::EntityBuilder* builder) const;

	virtual bool serialize(Serializer& s);

	Path& getPath() { return m_path; }

	const Path& getPath() const { return m_path; }

	const PathEntity::TimeMode getTimeMode() const { return m_timeMode; }

	const Ref< world::SpatialEntityData >& getEntityData() const { return m_entityData; }

private:
	Path m_path;
	PathEntity::TimeMode m_timeMode;
	Ref< world::SpatialEntityData > m_entityData;
};

	}
}

#endif	// traktor_animation_PathEntityData_H
