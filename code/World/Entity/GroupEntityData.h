/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_world_GroupEntityData_H
#define traktor_world_GroupEntityData_H

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

/*! \brief Group entity data.
 * \ingroup World
 */
class T_DLLCLASS GroupEntityData : public EntityData
{
	T_RTTI_CLASS;

public:
	void addEntityData(EntityData* entityData);

	void removeEntityData(EntityData* entityData);

	void removeAllEntityData();

	void setEntityData(const RefArray< EntityData >& entityData);

	RefArray< EntityData >& getEntityData();

	const RefArray< EntityData >& getEntityData() const;

	virtual void setTransform(const Transform& transform) T_OVERRIDE;
	
	virtual void serialize(ISerializer& s) T_OVERRIDE;
	
private:
	RefArray< EntityData > m_entityData;
};
	
	}
}

#endif	// traktor_world_GroupEntityData_H
