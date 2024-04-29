/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/Sound.h"
#include "Sound/Resound/BankBuffer.h"
#include "Sound/Resound/IGrain.h"

namespace traktor::sound
{
	namespace
	{

struct BankBufferCursor : public RefCountImpl< IAudioBufferCursor >
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
		m_grainIndex = 0;
		if (m_grainCursor)
			m_grainCursor->reset();
	}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.BankBuffer", BankBuffer, IAudioBuffer)

BankBuffer::BankBuffer(const RefArray< IGrain >& grains)
:	m_grains(grains)
{
}

void BankBuffer::updateCursor(IAudioBufferCursor* cursor) const
{
	BankBufferCursor* bankCursor = static_cast< BankBufferCursor* >(cursor);
	const IGrain* currentGrain = m_grains[bankCursor->m_grainIndex];
	currentGrain->updateCursor(bankCursor->m_grainCursor);
}

const IGrain* BankBuffer::getCurrentGrain(const IAudioBufferCursor* cursor) const
{
	const BankBufferCursor* bankCursor = static_cast< const BankBufferCursor* >(cursor);
	const IGrain* currentGrain = m_grains[bankCursor->m_grainIndex];
	return currentGrain->getCurrentGrain(bankCursor->m_grainCursor);
}

void BankBuffer::getActiveGrains(const IAudioBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const
{
	const BankBufferCursor* bankCursor = static_cast< const BankBufferCursor* >(cursor);
	if (bankCursor && bankCursor->m_grainCursor)
	{
		const IGrain* currentGrain = m_grains[bankCursor->m_grainIndex];
		currentGrain->getActiveGrains(bankCursor->m_grainCursor, outActiveGrains);
	}
}

Ref< IAudioBufferCursor > BankBuffer::createCursor() const
{
	if (m_grains.empty())
		return nullptr;

	Ref< BankBufferCursor > bankCursor = new BankBufferCursor();

	bankCursor->m_grainIndex = 0;
	bankCursor->m_grainCursor = m_grains[0]->createCursor();

	return bankCursor->m_grainCursor ? bankCursor : nullptr;
}

bool BankBuffer::getBlock(IAudioBufferCursor* cursor, const IAudioMixer* mixer, AudioBlock& outBlock) const
{
	BankBufferCursor* bankCursor = static_cast< BankBufferCursor* >(cursor);

	const int32_t ngrains = int32_t(m_grains.size());
	if (bankCursor->m_grainIndex >= ngrains)
		return false;

	Ref< IGrain > grain = m_grains[bankCursor->m_grainIndex];
	for (;;)
	{
		if (grain->getBlock(
			bankCursor->m_grainCursor,
			mixer,
			outBlock
		))
			break;

		if (++bankCursor->m_grainIndex >= ngrains)
			return false;

		grain = m_grains[bankCursor->m_grainIndex];
		bankCursor->m_grainCursor = grain->createCursor();
	}

	return true;
}

}
