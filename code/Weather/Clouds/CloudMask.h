/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Misc/AutoPtr.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace weather
	{

class T_DLLCLASS CloudMask : public Object
{
	T_RTTI_CLASS;

public:
	struct Sample
	{
		Sample()
		:	opacity(255)
		,	size(255)
		{
		}

		uint8_t opacity;
		uint8_t size;
	};

	CloudMask(int32_t size);

	int32_t getSize() const;

	Sample getSample(int32_t x, int32_t y) const;

private:
	friend class CloudMaskFactory;

	int32_t m_size;
	AutoArrayPtr< Sample > m_data;
};

	}
}

