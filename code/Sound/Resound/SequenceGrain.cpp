/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/IAudioBuffer.h"
#include "Sound/Resound/SequenceGrain.h"

namespace traktor::sound
{
	namespace
	{

struct SequenceGrainCursor : public RefCountImpl< IAudioBufferCursor >
{
	int32_t m_grainIndex;
	Ref< IAudioBufferCursor > m_grainCursor;

	virtual void setParameter(handle_t id, float parameter) override final
	{
		if (m_grainCursor)
			m_grainCursor->setParameter(id, parameter);
	}

	virtual void disableRepeat() override final
	{
		if (m_grainCursor)
			m_grainCursor->disableRepeat();
	}

	virtual void reset() override final
	{
		if (m_grainCursor)
			m_grainCursor->reset();
	}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SequenceGrain", SequenceGrain, IGrain)

SequenceGrain::SequenceGrain(const RefArray< IGrain >& grains)
:	m_grains(grains)
{
}

Ref< IAudioBufferCursor > SequenceGrain::createCursor() const
{
	if (m_grains.empty())
		return nullptr;

	Ref< SequenceGrainCursor > cursor = new SequenceGrainCursor();
	cursor->m_grainIndex = 0;
	cursor->m_grainCursor = m_grains[0]->createCursor();

	return cursor->m_grainCursor ? cursor : nullptr;
}

void SequenceGrain::updateCursor(IAudioBufferCursor* cursor) const
{
	SequenceGrainCursor* sequenceCursor = static_cast< SequenceGrainCursor* >(cursor);
	const IGrain* grain = m_grains[sequenceCursor->m_grainIndex];
	return grain->updateCursor(sequenceCursor->m_grainCursor);
}

const IGrain* SequenceGrain::getCurrentGrain(const IAudioBufferCursor* cursor) const
{
	const SequenceGrainCursor* sequenceCursor = static_cast< const SequenceGrainCursor* >(cursor);
	const IGrain* grain = m_grains[sequenceCursor->m_grainIndex];
	return grain->getCurrentGrain(sequenceCursor->m_grainCursor);
}

void SequenceGrain::getActiveGrains(const IAudioBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const
{
	const SequenceGrainCursor* sequenceCursor = static_cast< const SequenceGrainCursor* >(cursor);
	const IGrain* grain = m_grains[sequenceCursor->m_grainIndex];

	outActiveGrains.push_back(this);

	grain->getActiveGrains(sequenceCursor->m_grainCursor, outActiveGrains);
}

bool SequenceGrain::getBlock(IAudioBufferCursor* cursor, const IAudioMixer* mixer, AudioBlock& outBlock) const
{
	SequenceGrainCursor* sequenceCursor = static_cast< SequenceGrainCursor* >(cursor);

	int32_t ngrains = int32_t(m_grains.size());
	if (sequenceCursor->m_grainIndex >= ngrains)
		return false;

	IGrain* grain = m_grains[sequenceCursor->m_grainIndex];

	for (;;)
	{
		if (grain->getBlock(
			sequenceCursor->m_grainCursor,
			mixer,
			outBlock
		))
			break;

		if (++sequenceCursor->m_grainIndex >= ngrains)
			return false;

		grain = m_grains[sequenceCursor->m_grainIndex];

		sequenceCursor->m_grainCursor = grain->createCursor();
	}

	return true;
}

}
