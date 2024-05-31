/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::runtime
{

/*! Log statement from running target.
 * \ingroup Runtime
 */
class T_DLLCLASS TargetLog : public ISerializable
{
	T_RTTI_CLASS;

public:
	TargetLog() = default;

	explicit TargetLog(uint32_t threadId, int32_t level, const std::wstring& text);

	uint32_t getThreadId() const { return m_threadId; }

	int32_t getLevel() const { return m_level; }

	const std::wstring& getText() const { return m_text; }

	virtual void serialize(ISerializer& s) override final;

private:
	uint32_t m_threadId = 0;
	int32_t m_level = 0;
	std::wstring m_text;
};

}
