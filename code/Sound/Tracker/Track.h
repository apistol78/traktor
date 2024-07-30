/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class IEvent;
class Play;

class T_DLLCLASS Track : public Object
{
	T_RTTI_CLASS;

public:
	struct Key
	{
		int32_t at;
		Ref< Play > play;
		RefArray< IEvent > events;
	};

	const Key* findKey(int32_t position) const;

private:
	friend class TrackData;

	AlignedVector< Key > m_keys;
};

}
