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

/*! Event from target when debug state has changed.
 * \ingroup Runtime
 */
class T_DLLCLASS ScriptDebuggerStateChange : public ISerializable
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerStateChange() = default;

	explicit ScriptDebuggerStateChange(bool running);

	bool isRunning() const { return m_running;  }

	virtual void serialize(ISerializer& s) override final;

private:
	bool m_running = false;
};

}
