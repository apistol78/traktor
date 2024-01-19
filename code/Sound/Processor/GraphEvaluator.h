/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Containers/StaticVector.h"
#include "Core/Timer/Timer.h"
#include "Sound/Types.h"
#include "Sound/Processor/ProcessorTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class Graph;
class IAudioMixer;
class InputPin;
class IAudioBufferCursor;
class Node;
class OutputPin;
struct AudioBlock;

class T_DLLCLASS GraphEvaluator : public Object
{
	T_RTTI_CLASS;

public:
	bool create(const Graph* graph);

	void setParameter(handle_t id, float parameter);

	bool evaluateScalar(const OutputPin* producerPin, float& outScalar) const;

	bool evaluateScalar(const InputPin* consumerPin, float& outScalar) const;

	bool evaluateBlock(const OutputPin* producerPin, const IAudioMixer* mixer, AudioBlock& outBlock) const;

	bool evaluateBlock(const InputPin* consumerPin, const IAudioMixer* mixer, AudioBlock& outBlock) const;

	NodePinType evaluatePinType(const InputPin* consumerPin) const;

	float getTime() const;

	void flushCachedBlocks();

private:
	Ref< const Graph > m_graph;
	SmallMap< const Node*, Ref< IAudioBufferCursor > > m_nodeCursors;
	Timer m_timer;

	mutable StaticVector< AudioBlock, 128 > m_blocks;
	mutable SmallMap< const OutputPin*, AudioBlock > m_cachedBlocks;

	AudioBlock* copyBlock(const AudioBlock& sourceBlock) const;
};

}
