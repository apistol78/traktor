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

class T_DLLCLASS GotoEventData : public IEventData
{
	T_RTTI_CLASS;

public:
	GotoEventData() = default;

	explicit GotoEventData(int32_t pattern, int32_t row);

	virtual Ref< IEvent > createInstance() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	int32_t m_pattern = 0;
	int32_t m_row = 0;
};

}
