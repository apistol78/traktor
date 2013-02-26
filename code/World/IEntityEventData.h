#ifndef traktor_world_IEntityEventData_H
#define traktor_world_IEntityEventData_H

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

class IEntityBuilder;
class IEntityEvent;

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS IEntityEventData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< IEntityEvent > create(const IEntityBuilder* entityBuilder) const = 0;
};

	}
}

#endif	// traktor_world_IEntityEventData_H
