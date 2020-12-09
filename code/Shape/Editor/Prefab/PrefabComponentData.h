#pragma once

#include "World/Entity/GroupComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace shape
	{

class EntityData;

/*! Prefab component data.
 * \ingroup Shape
 */
class T_DLLCLASS PrefabComponentData : public world::GroupComponentData
{
	T_RTTI_CLASS;
};

	}
}
