/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

/*! Call stack debug information.
 * \ingroup Script
 */
class T_DLLCLASS StackFrame : public ISerializable
{
	T_RTTI_CLASS;

public:
	StackFrame();

	void setScriptId(const Guid& scriptId);

	const Guid& getScriptId() const;

	void setFunctionName(const std::wstring& functionName);

	const std::wstring& getFunctionName() const;

	void setLine(uint32_t line);

	uint32_t getLine() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_scriptId;
	std::wstring m_functionName;
	uint32_t m_line;
};

	}
}

