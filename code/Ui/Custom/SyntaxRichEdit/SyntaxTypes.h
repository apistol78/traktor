/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_SyntaxTypes_H
#define traktor_ui_custom_SyntaxTypes_H

#include "Core/Config.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

enum SyntaxOutlineType
{
	SotUnknown,
	SotVariable,
	SotFunction
};

struct SyntaxOutline
{
	SyntaxOutlineType type;
	int32_t line;
	std::wstring name;

	SyntaxOutline()
	:	type(SotUnknown)
	,	line(0)
	{
	}

	SyntaxOutline(SyntaxOutlineType type_, int32_t line_, const std::wstring& name_)
	:	type(type_)
	,	line(line_)
	,	name(name_)
	{
	}
};

		}
	}
}

#endif	// traktor_ui_custom_SyntaxTypes_H
