/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Sound/Processor/ImmutableNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class T_DLLCLASS Parameter : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Parameter();

	virtual bool bind(resource::IResourceManager* resourceManager) override final;

	virtual Ref< ISoundBufferCursor > createCursor() const override final;

	virtual bool getScalar(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, float& outParameter) const override final;

	virtual bool getBlock(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, const IAudioMixer* mixer, SoundBlock& outBlock) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
	float m_defaultValue = 0.0f;
};

	}
}
