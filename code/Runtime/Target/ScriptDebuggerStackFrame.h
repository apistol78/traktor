/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::script
{

class StackFrame;

}

namespace traktor::runtime
{

/*! Response from running target when a stack frame has been captured.
 * \ingroup Runtime
 */
class T_DLLCLASS ScriptDebuggerStackFrame : public ISerializable
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerStackFrame() = default;

	explicit ScriptDebuggerStackFrame(script::StackFrame* frame);

	script::StackFrame* getFrame() const { return m_frame; }

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< script::StackFrame > m_frame;
};

}
