#pragma once

#include <string>
#include "Core/Config.h"

namespace traktor
{
	namespace ui
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

