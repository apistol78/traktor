/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

/*! Debugger status response.
 * \ingroup Runtime
 */
class T_DLLCLASS ScriptDebuggerStatus : public ISerializable
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerStatus() = default;

	explicit ScriptDebuggerStatus(bool running);

	bool isRunning() const { return m_running; }

	virtual void serialize(ISerializer& s) override final;

private:
	bool m_running = false;
};

}
