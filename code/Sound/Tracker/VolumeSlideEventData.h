/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Sound/Tracker/IEventData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class T_DLLCLASS VolumeSlideEventData : public IEventData
{
	T_RTTI_CLASS;

public:
	VolumeSlideEventData() = default;

	explicit VolumeSlideEventData(float amount);

	virtual Ref< IEvent > createInstance() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	float m_amount = 0.0f;
};

}
