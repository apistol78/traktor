/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_EditValidator_H
#define traktor_ui_EditValidator_H

#include <string>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief Text edit validator.
 * \ingroup UI
 */
class T_DLLCLASS EditValidator : public Object
{
	T_RTTI_CLASS;

public:
	enum Result
	{
		VrOk,
		VrIncomplete,
		VrInvalid
	};

	virtual Result validate(const std::wstring& text) const = 0;
};

	}
}

#endif	// traktor_ui_EditValidator_H
