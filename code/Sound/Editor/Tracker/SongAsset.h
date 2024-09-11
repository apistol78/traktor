/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class PatternData;

class T_DLLCLASS SongAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setBpm(int32_t bpm);

	int32_t getBpm() const { return m_bpm; }

	void addPattern(const PatternData* pattern);

	const RefArray< const PatternData >& getPatterns() const { return m_patterns; }

	virtual void serialize(ISerializer& s) override final;

private:
	friend class SongPipeline;

	Guid m_category;
	float m_presence = 0.0f;
	float m_presenceRate = 0.25f;
	int32_t m_bpm = 125;
	RefArray< const PatternData > m_patterns;
};

}
