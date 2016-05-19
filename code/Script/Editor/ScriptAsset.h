#ifndef traktor_script_ScriptAsset_H
#define traktor_script_ScriptAsset_H

#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

/*! \brief External script asset.
 * \ingroup Script
 */
class T_DLLCLASS ScriptAsset : public editor::Asset
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_script_ScriptAsset_H
