#ifndef traktor_editor_BrowseFilter_H
#define traktor_editor_BrowseFilter_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace editor
	{

/*! \brief Browse instance filter. */
class T_DLLCLASS BrowseFilter
{
public:
	virtual bool acceptable(db::Instance* instance) const = 0;
};

	}
}

#endif	// traktor_editor_BrowseFilter_H
