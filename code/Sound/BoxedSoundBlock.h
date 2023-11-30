/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
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
class T_DLLCLASS BoxedSoundBlock : public Boxed
{
	T_RTTI_CLASS;

public:
	explicit BoxedSoundBlock(SoundBlock& value);

	float get(uint32_t channel, uint32_t index) const;

	void set(uint32_t channel, uint32_t index, float sample);

	uint32_t getSamplesCount() const { return m_value.samplesCount; }

	uint32_t getSampleRate() const { return m_value.sampleRate; }

	uint32_t getMaxChannel() const { return m_value.maxChannel; }

	SoundBlock& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	SoundBlock& m_value;
};

}

namespace traktor
{

/*!
 * \ingroup Sound
 */
template < >
struct CastAny < sound::SoundBlock&, false >
{
	static std::wstring typeName() {
		return L"traktor.sound.SoundBlock";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< sound::BoxedSoundBlock >(value.getObjectUnsafe());
	}
	static Any set(sound::SoundBlock& value) {
		return Any::fromObject(new sound::BoxedSoundBlock(value));
	}
	static sound::SoundBlock& get(const Any& value) {
		return static_cast< sound::BoxedSoundBlock* >(value.getObject())->unbox();
	}
};

}
