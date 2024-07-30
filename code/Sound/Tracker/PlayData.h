/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::sound
{

class Play;
class Sound;

class T_DLLCLASS PlayData : public ISerializable
{
	T_RTTI_CLASS;

public:
	PlayData() = default;

	explicit PlayData(const resource::Id< Sound >& sound, int32_t note, int32_t repeatFrom, int32_t repeatLength);

	Ref< Play > createInstance(resource::IResourceManager* resourceManager) const;

	const resource::Id< Sound >& getSound() const { return m_sound; }

	int32_t getNote() const { return m_note; }

	virtual void serialize(ISerializer& s) override final;

private:
	resource::Id< Sound > m_sound;
	int32_t m_note = 57;
	int32_t m_repeatFrom = 0;
	int32_t m_repeatLength = 0;
};

}
