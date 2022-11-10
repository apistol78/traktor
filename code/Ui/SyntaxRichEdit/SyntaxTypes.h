/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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

