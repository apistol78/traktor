/*
 * TRAKTOR
 * Copyright (c) 2023-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Class/Boxed.h"
#include "Core/Class/CastAny.h"
#include "Sound/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

/*!
 * \ingroup Sound
 */
class T_DLLCLASS BoxedAudioBlock : public Boxed
{
	T_RTTI_CLASS;

public:
	explicit BoxedAudioBlock(AudioBlock& value);

	float get(uint32_t channel, uint32_t index) const;

	void set(uint32_t channel, uint32_t index, float sample);

	uint32_t getSamplesCount() const { return m_value.samplesCount; }

	uint32_t getSampleRate() const { return m_value.sampleRate; }

	uint32_t getMaxChannel() const { return m_value.maxChannel; }

	AudioBlock& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	AudioBlock& m_value;
};

}

namespace traktor
{

/*!
 * \ingroup Sound
 */
template < >
struct CastAny < sound::AudioBlock&, false >
{
	static std::wstring typeName() {
		return L"traktor.sound.AudioBlock";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< sound::BoxedAudioBlock >(value.getObjectUnsafe());
	}
	static Any set(sound::AudioBlock& value) {
		return Any::fromObject(new sound::BoxedAudioBlock(value));
	}
	static sound::AudioBlock& get(const Any& value) {
		return static_cast< sound::BoxedAudioBlock* >(value.getObject())->unbox();
	}
};

}
