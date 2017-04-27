/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_script_ScriptOutlineLua_H
#define traktor_script_ScriptOutlineLua_H

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
	virtual Ref< Node > parse(const std::wstring& text) const T_OVERRIDE T_FINAL;

	void parseResult(Node* node) const;

	void syntaxError() const;

private:
	mutable Ref< Node > m_result;
};

	}
}

#endif	// traktor_script_ScriptOutlineLua_H
