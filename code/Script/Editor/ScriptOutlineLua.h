#pragma once

#include "Script/Editor/IScriptOutline.h"

namespace traktor
{
	namespace script
	{

/*! \brief LUA script outline parser.
 * \ingroup Script
 */
class ScriptOutlineLua : public IScriptOutline
{
	T_RTTI_CLASS;

public:
	virtual Ref< Node > parse(const std::wstring& text) const override final;

	void parseResult(Node* node) const;

	void syntaxError() const;

private:
	mutable Ref< Node > m_result;
};

	}
}

