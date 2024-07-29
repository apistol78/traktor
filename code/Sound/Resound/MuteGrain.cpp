/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Timer/Timer.h"
#include "Sound/IAudioBuffer.h"
#include "Sound/Resound/MuteGrain.h"

namespace traktor::sound
{
	namespace
	{

struct MuteGrainCursor : public RefCountImpl< IAudioBufferCursor >
{
	Timer m_timer;
	double m_end;

	virtual void setParameter(handle_t id, float parameter)  override final {}

	virtual void disableRepeat() override final {}

	virtual void reset() override final {}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.MuteGrain", MuteGrain, IGrain)

MuteGrain::MuteGrain(double duration)
:	m_duration(duration)
{
}

Ref< IAudioBufferCursor > MuteGrain::createCursor() const
{
	Ref< MuteGrainCursor > muteCursor = new MuteGrainCursor();
	muteCursor->m_end = muteCursor->m_timer.getElapsedTime() + m_duration;
	return muteCursor;
}

void MuteGrain::updateCursor(IAudioBufferCursor* cursor) const
{
}

const IGrain* MuteGrain::getCurrentGrain(const IAudioBufferCursor* cursor) const
{
	return this;
}

void MuteGrain::getActiveGrains(const IAudioBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const
{
	outActiveGrains.push_back(this);
}

bool MuteGrain::getBlock(IAudioBufferCursor* cursor, const IAudioMixer* mixer, AudioBlock& outBlock) const
{
	MuteGrainCursor* muteCursor = static_cast< MuteGrainCursor* >(cursor);
	return muteCursor->m_timer.getElapsedTime() <= muteCursor->m_end;
}

}
