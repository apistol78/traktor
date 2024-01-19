/*
 * TRAKTOR
 * Copyright (c) 2023-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/BoxedAllocator.h"
#include "Sound/BoxedAudioBlock.h"

namespace traktor::sound
{
	namespace
	{
	
BoxedAllocator< BoxedAudioBlock, 64 > s_allocBoxedAudioBlock;
	
	}
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.AudioBlock", BoxedAudioBlock, Boxed)

BoxedAudioBlock::BoxedAudioBlock(AudioBlock& value)
:	m_value(value)
{
}

float BoxedAudioBlock::get(uint32_t channel, uint32_t index) const
{
	return m_value.samples[channel][index];
}

void BoxedAudioBlock::set(uint32_t channel, uint32_t index, float sample)
{
	m_value.samples[channel][index] = sample;
}

std::wstring BoxedAudioBlock::toString() const
{
	return L"(sound block)";
}

void* BoxedAudioBlock::operator new (size_t size)
{
	return s_allocBoxedAudioBlock.alloc();
}

void BoxedAudioBlock::operator delete (void* ptr)
{
	s_allocBoxedAudioBlock.free(ptr);
}

}
