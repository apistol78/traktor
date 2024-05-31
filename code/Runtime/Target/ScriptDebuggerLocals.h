/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
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

class Variable;

}

namespace traktor::runtime
{

/*! Response from running target when locals has been captured.
 * \ingroup Runtime
 */
class T_DLLCLASS ScriptDebuggerLocals : public ISerializable
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerLocals() = default;

	explicit ScriptDebuggerLocals(const RefArray< script::Variable >& locals);

	const RefArray< script::Variable >& getLocals() const { return m_locals; }

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< script::Variable > m_locals;
};

}
