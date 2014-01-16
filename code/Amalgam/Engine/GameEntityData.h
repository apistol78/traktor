#ifndef traktor_amalgam_GameEntityData_H
#define traktor_amalgam_GameEntityData_H

#include "World/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class EntityEventSetData;

	}

	namespace amalgam
	{

/*! \brief
 * \ingroup Amalgam
 */
class T_DLLCLASS GameEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s);

private:
	friend class GameEntityFactory;

	std::wstring m_tag;
	Ref< ISerializable > m_object;
	Ref< world::EntityData > m_entityData;
	Ref< world::EntityEventSetData > m_eventSetData;
};

	}
}

#endif	// traktor_amalgam_GameEntityData_H
