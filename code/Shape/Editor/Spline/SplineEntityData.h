#pragma once

#include "World/EntityData.h"

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

/*!
 * \ingroup Shape
 */
class T_DLLCLASS SplineEntityData : public world::EntityData
{
	T_RTTI_CLASS;
};

	}
}
