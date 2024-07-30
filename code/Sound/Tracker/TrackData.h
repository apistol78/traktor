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
#include "Core/Containers/AlignedVector.h"
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

class IEventData;
class PlayData;
class Track;

class T_DLLCLASS TrackData : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct T_DLLCLASS Key
	{
		int32_t at = 0;
		Ref< PlayData > play;
		RefArray< IEventData > events;

		void serialize(ISerializer& s);
	};

	Ref< Track > createInstance(resource::IResourceManager* resourceManager) const;

	void addKey(const Key& key);

	const Key* findKey(int32_t position) const;

	const AlignedVector< Key >& getKeys() const { return m_keys; }

	virtual void serialize(ISerializer& s) override final;

private:
	AlignedVector< Key > m_keys;
};

	}
}
