#pragma once

#include "Core/RefArray.h"
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

class EntityData;

/*! Facade component persistent data.
 * \ingroup World
 */
class T_DLLCLASS FacadeComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	void addEntityData(EntityData* entityData);

	void removeEntityData(EntityData* entityData);

	void removeAllEntityData();

	void setEntityData(const RefArray< EntityData >& entityData);

	RefArray< EntityData >& getEntityData();

	const RefArray< EntityData >& getEntityData() const;

	const std::wstring& getShow() const;

	virtual void serialize(ISerializer& s) override final;

private:
    RefArray< EntityData > m_entityData;
	std::wstring m_show;
};

	}
}
