#ifndef traktor_script_IScriptBlob_H
#define traktor_script_IScriptBlob_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

/*! \brief Pre-compiled script blob.
 * \ingroup Script
 */
class T_DLLCLASS IScriptBlob : public ISerializable
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_script_IScriptBlob_H
