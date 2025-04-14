/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::script
{

/*! Add or remove script breakpoint on running target.
 * \ingroup Runtime
 */
class T_DLLCLASS ScriptDebuggerBreakpoint : public ISerializable
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerBreakpoint() = default;

	explicit ScriptDebuggerBreakpoint(bool add, const std::wstring& fileName, uint32_t lineNumber);

	bool shouldAdd() const { return m_add; }

	const std::wstring& getFileName() const { return m_fileName; }

	uint32_t getLineNumber() const { return m_lineNumber; }

	virtual void serialize(ISerializer& s) override final;

private:
	bool m_add = false;
	std::wstring m_fileName;
	uint32_t m_lineNumber = 0;
};

}
