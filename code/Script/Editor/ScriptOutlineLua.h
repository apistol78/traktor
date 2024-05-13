/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Script/Editor/IScriptOutline.h"

namespace traktor::script
{

/*! LUA script outline parser.
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
