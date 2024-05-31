/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::runtime
{

/*! Script profiler measurement from running target.
 * \ingroup Runtime
 */
class T_DLLCLASS ScriptProfilerCallMeasured : public ISerializable
{
	T_RTTI_CLASS;

public:
	ScriptProfilerCallMeasured() = default;

	explicit ScriptProfilerCallMeasured(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration);

	const Guid& getScriptId() const { return m_scriptId; }

	const std::wstring& getFunction() const { return m_function; }

	uint32_t getCallCount() const { return m_callCount; }

	double getInclusiveDuration() const { return m_inclusiveDuration; }

	double getExclusiveDuration() const { return m_exclusiveDuration; }

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_scriptId;
	std::wstring m_function;
	uint32_t m_callCount = 0;
	double m_inclusiveDuration = 0.0;
	double m_exclusiveDuration = 0.0;
};

}
