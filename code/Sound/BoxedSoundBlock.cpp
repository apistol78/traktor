/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/BoxedAllocator.h"
#include "Sound/BoxedSoundBlock.h"

namespace traktor::sound
{
	namespace
	{
	
BoxedAllocator< BoxedSoundBlock, 64 > s_allocBoxedSoundBlock;
	
	}
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundBlock", BoxedSoundBlock, Boxed)

BoxedSoundBlock::BoxedSoundBlock(SoundBlock& value)
:	m_value(value)
{
}

float BoxedSoundBlock::get(uint32_t channel, uint32_t index) const
{
	return m_value.samples[channel][index];
}

void BoxedSoundBlock::set(uint32_t channel, uint32_t index, float sample)
{
	m_value.samples[channel][index] = sample;
}

std::wstring BoxedSoundBlock::toString() const
{
	return L"(sound block)";
}

void* BoxedSoundBlock::operator new (size_t size)
{
	return s_allocBoxedSoundBlock.alloc();
}

void BoxedSoundBlock::operator delete (void* ptr)
{
	s_allocBoxedSoundBlock.free(ptr);
}

}
