/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <ctime>
#include "Sound/IAudioBuffer.h"
#include "Sound/Resound/RandomGrain.h"

namespace traktor::sound
{
	namespace
	{

struct RandomGrainCursor : public RefCountImpl< IAudioBufferCursor >
{
	Ref< IGrain > m_grain;
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.RandomGrain", RandomGrain, IGrain)

RandomGrain::RandomGrain(
	const RefArray< IGrain >& grains,
	bool humanize
)
:	m_grains(grains)
,	m_humanize(humanize)
,	m_random(uint32_t(clock()))
,	m_last(-1)
{
}

Ref< IAudioBufferCursor > RandomGrain::createCursor() const
{
	if (m_grains.empty())
		return 0;

	int32_t index = int32_t(m_random.nextFloat() * (m_grains.size() - 1) + 0.5f);
	if (m_humanize && m_grains.size() >= 2)
	{
		while (index == m_last)
			index = int32_t(m_random.nextFloat() * (m_grains.size() - 1) + 0.5f);
		m_last = index;
	}
	T_ASSERT(index >= 0);
	T_ASSERT(index < m_grains.size());

	Ref< RandomGrainCursor > cursor = new RandomGrainCursor();
	cursor->m_grain = m_grains[index];
	cursor->m_grainCursor = m_grains[index]->createCursor();

	return cursor->m_grainCursor ? cursor : 0;
}

void RandomGrain::updateCursor(IAudioBufferCursor* cursor) const
{
	RandomGrainCursor* randomCursor = static_cast< RandomGrainCursor* >(cursor);
	return randomCursor->m_grain->updateCursor(randomCursor->m_grainCursor);
}

const IGrain* RandomGrain::getCurrentGrain(const IAudioBufferCursor* cursor) const
{
	const RandomGrainCursor* randomCursor = static_cast< const RandomGrainCursor* >(cursor);
	return randomCursor->m_grain->getCurrentGrain(randomCursor->m_grainCursor);
}

void RandomGrain::getActiveGrains(const IAudioBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const
{
	const RandomGrainCursor* randomCursor = static_cast< const RandomGrainCursor* >(cursor);
	T_ASSERT(randomCursor);

	outActiveGrains.push_back(this);

	randomCursor->m_grain->getActiveGrains(randomCursor->m_grainCursor, outActiveGrains);
}

bool RandomGrain::getBlock(IAudioBufferCursor* cursor, const IAudioMixer* mixer, AudioBlock& outBlock) const
{
	RandomGrainCursor* randomCursor = static_cast< RandomGrainCursor* >(cursor);
	return randomCursor->m_grain->getBlock(
		randomCursor->m_grainCursor,
		mixer,
		outBlock
	);
}

}
