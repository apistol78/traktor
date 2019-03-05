#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace runtime
	{

/*! \brief
 * \ingroup Runtime
 */
class T_DLLCLASS IRemoteEvent : public ISerializable
{
	T_RTTI_CLASS;
};

	}
}

