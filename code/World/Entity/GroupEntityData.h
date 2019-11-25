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

/*! Group entity data.
 * \ingroup World
 */
class T_DLLCLASS GroupEntityData : public EntityData
{
	T_RTTI_CLASS;

public:
	GroupEntityData();

	void setMask(uint32_t mask);

	uint32_t getMask() const;

	void addEntityData(EntityData* entityData);

	void removeEntityData(EntityData* entityData);

	void removeAllEntityData();

	void setEntityData(const RefArray< EntityData >& entityData);

	RefArray< EntityData >& getEntityData();

	const RefArray< EntityData >& getEntityData() const;

	virtual void setTransform(const Transform& transform) override;

	virtual void serialize(ISerializer& s) override;

private:
	uint32_t m_mask;
	RefArray< EntityData > m_entityData;
};

	}
}

