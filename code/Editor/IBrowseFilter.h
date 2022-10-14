#pragma once

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{

class Instance;

}

namespace traktor::editor
{

/*! Browse instance filter interface.
 * \ingroup Editor
 */
class T_DLLCLASS IBrowseFilter
{
public:
	virtual bool acceptable(db::Instance* instance) const = 0;
};

}
