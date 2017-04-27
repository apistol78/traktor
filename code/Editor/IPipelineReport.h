/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_IPipelineReport_H
#define traktor_editor_IPipelineReport_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

/*! \brief Pipeline reporting interface.
 * \ingroup Editor.
 */
class T_DLLCLASS IPipelineReport : public Object
{
	T_RTTI_CLASS;

public:
	virtual void set(const std::wstring& name, int32_t value) = 0;

	virtual void set(const std::wstring& name, const std::wstring& value) = 0;
};

	}
}

#endif	// traktor_editor_IPipelineReport_H
