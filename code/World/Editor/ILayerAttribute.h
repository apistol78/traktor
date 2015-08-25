#ifndef traktor_world_ILayerAttribute_H
#define traktor_world_ILayerAttribute_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Layer atttribute base class.
 * \ingroup World
 */
class T_DLLCLASS ILayerAttribute : public ISerializable
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_world_ILayerAttribute_H
