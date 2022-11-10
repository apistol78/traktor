/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace sound
	{

class Pattern;
class TrackData;

class T_DLLCLASS PatternData : public ISerializable
{
	T_RTTI_CLASS;

public:
	PatternData();

	PatternData(int32_t duration);

	Ref< Pattern > createInstance(resource::IResourceManager* resourceManager) const;

	int32_t getDuration() const { return m_duration; }

	void addTrack(const TrackData* track);

	const RefArray< const TrackData >& getTracks() const { return m_tracks; }

	virtual void serialize(ISerializer& s) override final;

private:
	int32_t m_duration;
	RefArray< const TrackData > m_tracks;
};

	}
}
