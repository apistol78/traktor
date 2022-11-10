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
#include "Core/Ref.h"
#include "Script/Editor/IScriptOutline.h"

namespace traktor
{
	namespace script
	{

struct LuaGrammarToken
{
	std::wstring text;
	Ref< IScriptOutline::Node > node;
	int32_t line;

	LuaGrammarToken()
	:	line(0)
	{
	}

	LuaGrammarToken(const std::wstring& text_, int32_t line_)
	:	text(text_)
	,	line(line_)
	{
	}

	LuaGrammarToken(IScriptOutline::Node* node_, int32_t line_)
	:	node(node_)
	,	line(line_)
	{
	}

	LuaGrammarToken(const std::wstring& text_, IScriptOutline::Node* node_, int32_t line_)
	:	text(text_)
	,	node(node_)
	,	line(line_)
	{
	}
};

	}
}

