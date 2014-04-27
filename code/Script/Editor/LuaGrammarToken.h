#ifndef traktor_script_LuaGrammarToken_H
#define traktor_script_LuaGrammarToken_H

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

#endif	// traktor_script_LuaGrammarToken_H
