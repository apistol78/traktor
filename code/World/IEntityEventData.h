#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

/*! Entity event data.
 * \ingroup World
 */
class T_DLLCLASS IEntityEventData : public ISerializable
{
	T_RTTI_CLASS;
};

}
