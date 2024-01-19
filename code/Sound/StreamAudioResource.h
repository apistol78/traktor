/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Sound/IAudioResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

/*! Stream audio resource.
 * \ingroup Sound
 */
class T_DLLCLASS StreamAudioResource : public IAudioResource
{
	T_RTTI_CLASS;

public:
	virtual Ref< Sound > createSound(resource::IResourceManager* resourceManager, const db::Instance* resourceInstance) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class SoundPipeline;

	const TypeInfo* m_decoderType = nullptr;
	std::wstring m_category;
	float m_gain = 0.0f;
	float m_range = 0.0f;
	bool m_preload = false;
};

}
