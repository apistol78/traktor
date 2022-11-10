/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Sound/ISoundResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class PatternData;

class T_DLLCLASS SongResource : public ISoundResource
{
	T_RTTI_CLASS;

public:
	SongResource();

	SongResource(
		const RefArray< const PatternData >& patterns,
		const std::wstring& category,
		float gain,
		float range,
		int32_t bpm
	);

	virtual Ref< Sound > createSound(resource::IResourceManager* resourceManager, const db::Instance* resourceInstance) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< const PatternData > m_patterns;
	std::wstring m_category;
	float m_gain;
	float m_range;
	int32_t m_bpm;
};

	}
}
