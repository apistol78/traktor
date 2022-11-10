/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Sound/ISoundBuffer.h"

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

class Graph;

/*! GraphBuffer instance.
 */
class T_DLLCLASS GraphBuffer : public ISoundBuffer
{
	T_RTTI_CLASS;

public:
	GraphBuffer(const Graph* graph);

	virtual Ref< ISoundBufferCursor > createCursor() const override final;

	virtual bool getBlock(ISoundBufferCursor* cursor, const IAudioMixer* mixer, SoundBlock& outBlock) const override final;

private:
	Ref< const Graph > m_graph;
};

	}
}
