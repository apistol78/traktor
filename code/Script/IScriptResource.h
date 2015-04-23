#ifndef traktor_script_IScriptResource_H
#define traktor_script_IScriptResource_H

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

/*! \brief Pre-compiled script resource.
 * \ingroup Script
 */
class T_DLLCLASS IScriptResource : public ISerializable
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_script_IScriptResource_H
