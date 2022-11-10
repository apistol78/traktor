/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
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

/*! Static sound resource.
 * \ingroup Sound
 */
class T_DLLCLASS StaticSoundResource : public ISoundResource
{
	T_RTTI_CLASS;

public:
	StaticSoundResource();

	virtual Ref< Sound > createSound(resource::IResourceManager* resourceManager, const db::Instance* resourceInstance) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class SoundPipeline;

	std::wstring m_category;
	uint32_t m_sampleRate;
	uint32_t m_samplesCount;
	uint32_t m_channelsCount;
	float m_gain;
	float m_range;
	const TypeInfo* m_decoderType;
};

	}
}

