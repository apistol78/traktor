/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_IBrowseFilter_H
#define traktor_editor_IBrowseFilter_H

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

/*! \brief Browse instance filter interface.
 * \ingroup Editor
 */
class T_DLLCLASS IBrowseFilter
{
public:
	virtual bool acceptable(db::Instance* instance) const = 0;
};

	}
}

#endif	// traktor_editor_IBrowseFilter_H
