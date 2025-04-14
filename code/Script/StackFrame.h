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

/*! Call stack debug information.
 * \ingroup Script
 */
class T_DLLCLASS StackFrame : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setFileName(const std::wstring& fileName);

	const std::wstring& getFileName() const;

	void setFunctionName(const std::wstring& functionName);

	const std::wstring& getFunctionName() const;

	void setLine(uint32_t line);

	uint32_t getLine() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_fileName;
	std::wstring m_functionName;
	uint32_t m_line = 0;
};

}
