#ifndef traktor_amalgam_IRemoteEvent_H
#define traktor_amalgam_IRemoteEvent_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{
	
/*! \brief
 * \ingroup Amalgam
 */
class T_DLLCLASS IRemoteEvent : public ISerializable
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_amalgam_IRemoteEvent_H
