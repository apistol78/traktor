/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/MathUtils.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/ISoundMixer.h"
#include "Sound/Resound/InLoopOutGrain.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct InLoopOutGrainCursor : public RefCountImpl< ISoundBufferCursor >
{
	handle_t m_id;
	bool m_parameter;
	bool m_repeat;
	Ref< IGrain > m_grain;
	Ref< IGrain > m_loopGrain;
	Ref< ISoundBufferCursor > m_cursor;
	Ref< ISoundBufferCursor > m_loopCursor;

	virtual void setParameter(handle_t id, float parameter) T_OVERRIDE T_FINAL
	{
		if (m_cursor)
			m_cursor->setParameter(id, parameter);

		if (id == m_id)
			m_parameter = bool(parameter >= 0.5f);
	}

	virtual void disableRepeat() T_OVERRIDE T_FINAL
	{
		m_repeat = false;
	}

	virtual void reset() T_OVERRIDE T_FINAL
	{
		if (m_cursor)
			m_cursor->reset();
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.InLoopOutGrain", InLoopOutGrain, IGrain)

InLoopOutGrain::InLoopOutGrain(
	handle_t id,
	bool initial,
	IGrain* inGrain,
	IGrain* inLoopGrain,
	IGrain* outGrain,
	IGrain* outLoopGrain
)
:	m_id(id)
,	m_initial(initial)
,	m_inGrain(inGrain)
,	m_inLoopGrain(inLoopGrain)
,	m_outGrain(outGrain)
,	m_outLoopGrain(outLoopGrain)
{
}

Ref< ISoundBufferCursor > InLoopOutGrain::createCursor() const
{
	Ref< InLoopOutGrainCursor > iloCursor = new InLoopOutGrainCursor();

	iloCursor->m_id = m_id;
	iloCursor->m_parameter = m_initial;
	iloCursor->m_repeat = true;
	iloCursor->m_grain = m_initial ? m_inGrain : m_outGrain;
	iloCursor->m_cursor = iloCursor->m_grain->createCursor();
	iloCursor->m_loopGrain = m_initial ? m_inLoopGrain : m_outLoopGrain;
	iloCursor->m_loopCursor = iloCursor->m_loopGrain->createCursor();

	return iloCursor;
}

void InLoopOutGrain::updateCursor(ISoundBufferCursor* cursor) const
{
	InLoopOutGrainCursor* iloCursor = static_cast< InLoopOutGrainCursor* >(cursor);
	if (iloCursor->m_grain)
		iloCursor->m_grain->updateCursor(iloCursor->m_cursor);
	if (iloCursor->m_loopGrain)
		iloCursor->m_loopGrain->updateCursor(iloCursor->m_loopCursor);
}

const IGrain* InLoopOutGrain::getCurrentGrain(const ISoundBufferCursor* cursor) const
{
	return this;
}

void InLoopOutGrain::getActiveGrains(const ISoundBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const
{
	outActiveGrains.push_back(this);
}

bool InLoopOutGrain::getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const
{
	InLoopOutGrainCursor* iloCursor = static_cast< InLoopOutGrainCursor* >(cursor);

	if (
		(iloCursor->m_loopGrain == m_inLoopGrain && iloCursor->m_parameter == false) ||
		(iloCursor->m_loopGrain == m_outLoopGrain && iloCursor->m_parameter == true)
	)
	{
		if (!iloCursor->m_repeat)
			return false;

		iloCursor->m_grain = iloCursor->m_parameter ? m_inGrain : m_outGrain;
		iloCursor->m_cursor = iloCursor->m_grain->createCursor();
		iloCursor->m_loopGrain = iloCursor->m_parameter ? m_inLoopGrain : m_outLoopGrain;
		iloCursor->m_loopCursor = iloCursor->m_loopGrain->createCursor();
	}

	if (iloCursor->m_loopGrain)
	{
		if (!iloCursor->m_loopGrain->getBlock(iloCursor->m_loopCursor, mixer, outBlock))
		{
			if (!iloCursor->m_repeat)
				return false;

			iloCursor->m_loopCursor = iloCursor->m_loopGrain->createCursor();
		}
	}

	if (iloCursor->m_grain)
	{
		SoundBlock soundBlock = { { 0 }, outBlock.samplesCount, 0, 0 };
		if (iloCursor->m_grain->getBlock(iloCursor->m_cursor, mixer, soundBlock))
		{
			outBlock.sampleRate = max(outBlock.sampleRate, soundBlock.sampleRate);
			outBlock.samplesCount = min(outBlock.samplesCount, soundBlock.samplesCount);
			outBlock.maxChannel = max(outBlock.maxChannel, soundBlock.maxChannel);

			for (uint32_t i = 0; i < outBlock.maxChannel; ++i)
			{
				if (outBlock.samples[i] && soundBlock.samples[i])
				{
					mixer->addMulConst(
						outBlock.samples[i],
						soundBlock.samples[i],
						outBlock.samplesCount,
						1.0f
					);
				}
				else if (soundBlock.samples[i])
					outBlock.samples[i] = soundBlock.samples[i];
			}
		}
		else
		{
			iloCursor->m_grain = 0;
			iloCursor->m_cursor = 0;
		}
	}

	return true;
}

	}
}
