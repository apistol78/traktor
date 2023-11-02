/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::weather
{

class T_DLLCLASS CloudMaskResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	CloudMaskResource(int32_t size = 0);

	int32_t getSize() const;

	virtual void serialize(ISerializer& s) override final;

private:
	int32_t m_size;
};

}
