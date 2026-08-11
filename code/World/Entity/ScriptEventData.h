/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IEntityEventData.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

/*! Script event data.
 *
 * Event which call a method on the script component of the entity
 * issuing the event.
 *
 * \ingroup World
 */
class T_DLLCLASS ScriptEventData : public IEntityEventData
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

	/*! Name of method called on the sender's script component. */
	const std::wstring& getMethod() const { return m_method; }

private:
	std::wstring m_method;
};

}
