/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/MemberIdProxy.h"
#include "Sound/IAudioMixer.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/Sound.h"
#include "Sound/Processor/Nodes/Source.h"

namespace traktor::sound
{
	namespace
	{

const ImmutableNode::OutputPinDesc c_Source_o[] =
{
	{ L"Output", NodePinType::Signal },
	{ 0 }
};

class SourceCursor : public RefCountImpl< ISoundBufferCursor >
{
public:
	Ref< ISoundBuffer > m_soundBuffer;
	Ref< ISoundBufferCursor > m_soundCursor;

	virtual void setParameter(handle_t id, float parameter) override final
	{
		if (m_soundCursor)
			m_soundCursor->setParameter(id, parameter);
	}

	virtual void disableRepeat() override final
	{
	}

	virtual void reset() override final
	{
		if (m_soundCursor)
			m_soundCursor->reset();
	}
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Source", 0, Source, ImmutableNode)

Source::Source()
:	ImmutableNode(nullptr, c_Source_o)
{
}

Source::Source(const resource::IdProxy< Sound >& sound)
:	ImmutableNode(nullptr, c_Source_o)
,	m_sound(sound)
{
}

bool Source::bind(resource::IResourceManager* resourceManager)
{
	return resourceManager->bind(m_sound);
}

Ref< ISoundBufferCursor > Source::createCursor() const
{
	if (!m_sound)
		return nullptr;

	Ref< ISoundBuffer > soundBuffer = m_sound->getBuffer();
	if (!soundBuffer)
		return nullptr;

	Ref< ISoundBufferCursor > soundCursor = soundBuffer->createCursor();
	if (!soundCursor)
		return nullptr;

	Ref< SourceCursor > sourceCursor = new SourceCursor();
	sourceCursor->m_soundBuffer = soundBuffer;
	sourceCursor->m_soundCursor = soundCursor;

	return sourceCursor;
}

bool Source::getScalar(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const
{
	return false;
}

bool Source::getBlock(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, const IAudioMixer* mixer, SoundBlock& outBlock) const
{
	SourceCursor* sourceCursor = static_cast< SourceCursor* >(cursor);
	if (!sourceCursor)
		return false;

	T_ASSERT(sourceCursor->m_soundBuffer);
	T_ASSERT(sourceCursor->m_soundCursor);

	if (!sourceCursor->m_soundBuffer->getBlock(
		sourceCursor->m_soundCursor,
		mixer,
		outBlock
	))
		return false;

	if (m_sound->getGain() != 0.0f)
	{
		const float gain = decibelToLinear(m_sound->getGain());
		mixer->mulConst(
			outBlock.samples[0],
			outBlock.samplesCount,
			gain
		);
	}

	return true;
}

void Source::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	s >> resource::MemberIdProxy< Sound >(L"sound", m_sound);
}

}
