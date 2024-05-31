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
#include "Runtime/Target/IRemoteEvent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::runtime
{

/*! Application generic command events.
 * \ingroup Runtime
 *
 * These events are sent remotely from the editor to the
 * running target.
 */
class T_DLLCLASS CommandEvent : public IRemoteEvent
{
	T_RTTI_CLASS;

public:
	CommandEvent() = default;

	explicit CommandEvent(const std::wstring_view& function);

	const std::wstring& getFunction() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_function;
};

}
