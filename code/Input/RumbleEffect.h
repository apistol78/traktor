/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "Core/Serialization/ISerializable.h"
#include "Input/InputTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

/*! Rumble effect.
 * \ingroup Input
 */
class T_DLLCLASS RumbleEffect : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setRumbleKeys(const std::vector< std::pair< float, InputRumble > >& rumbleKeys);

	float getDuration() const;

	void getRumble(float time, InputRumble& outRumble) const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::vector< std::pair< float, InputRumble > > m_rumbleKeys;
};

	}
}

