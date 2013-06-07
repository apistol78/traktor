#ifndef traktor_world_EntityEventSetData_H
#define traktor_world_EntityEventSetData_H

#include <map>
#include "Core/Serialization/ISerializable.h"

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

class EntityEventSet;
class IEntityBuilder;
class IEntityEventData;

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS EntityEventSetData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< EntityEventSet > create(const IEntityBuilder* entityBuilder) const;

	virtual void serialize(ISerializer& s);

private:
	friend class EntityEventSetPipeline;

	std::map< std::wstring, Ref< IEntityEventData > > m_eventData;
};

	}
}

#endif	// traktor_world_EntityEventSetData_H
