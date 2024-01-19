/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Resource/IdProxy.h"
#include "Sound/Processor/ImmutableNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class Sound;

class T_DLLCLASS Source : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Source();

	explicit Source(const resource::IdProxy< Sound >& sound);

	virtual bool bind(resource::IResourceManager* resourceManager) override final;

	virtual Ref< IAudioBufferCursor > createCursor() const override final;

	virtual bool getScalar(IAudioBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const override final;

	virtual bool getBlock(IAudioBufferCursor* cursor, const GraphEvaluator* evaluator, const IAudioMixer* mixer, AudioBlock& outBlock) const override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::IdProxy< Sound >& getSound() const { return m_sound; }

private:
	resource::IdProxy< Sound > m_sound;
};

}
