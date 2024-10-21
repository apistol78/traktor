/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Vector4.h"
#include "Core/Memory/Alloc.h"
#include "Sound/IAudioBuffer.h"
#include "Sound/Processor/Graph.h"
#include "Sound/Processor/GraphEvaluator.h"
#include "Sound/Processor/Node.h"
#include "Sound/Processor/OutputPin.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.GraphEvaluator", GraphEvaluator, Object)

bool GraphEvaluator::create(const Graph* graph)
{
	m_graph = graph;
	for (auto node : graph->getNodes())
	{
		Ref< IAudioBufferCursor > nodeCursor = node->createCursor();
		if (!nodeCursor)
		{
			log::error << L"Node \"" << type_name(node) << L"\" failed; no cursor." << Endl;
			return false;
		}

		m_nodeCursors[node] = nodeCursor;
	}
	m_timer.reset();
	return true;
}

void GraphEvaluator::setParameter(handle_t id, float parameter)
{
	for (auto it : m_nodeCursors)
		it.second->setParameter(id, parameter);
}

bool GraphEvaluator::evaluateScalar(const OutputPin* producerPin, float& outScalar) const
{
	const Node* producerNode = producerPin->getNode();
	T_ASSERT(producerNode != nullptr);

	IAudioBufferCursor* producerCursor = m_nodeCursors[producerNode];
	if (!producerCursor)
		return false;

	if (!producerNode->getScalar(producerCursor, this, outScalar))
		return false;

	return true;
}

bool GraphEvaluator::evaluateScalar(const InputPin* consumerPin, float& outScalar) const
{
	const OutputPin* producerPin = m_graph->findSourcePin(consumerPin);
	if (producerPin)
		return evaluateScalar(producerPin, outScalar);
	else
		return false;
}

bool GraphEvaluator::evaluateBlock(const OutputPin* producerPin, const IAudioMixer* mixer, AudioBlock& outBlock) const
{
	const Node* producerNode = producerPin->getNode();
	T_ASSERT(producerNode != nullptr);

	IAudioBufferCursor* producerCursor = m_nodeCursors[producerNode];
	if (!producerCursor)
		return false;

	const uint32_t consumerCount = m_graph->getDestinationCount(producerPin);
	if (consumerCount == 1)
	{
		if (!producerNode->getBlock(producerCursor, this, mixer, outBlock))
			return false;
	}
	else if (consumerCount >= 2)
	{
		// Need to buffer output of node since we have multiple consumers.
		auto it = m_cachedBlocks.find(producerPin);
		if (it != m_cachedBlocks.end())
		{
			if (consumerCount == 2)
				outBlock = it->second;
			else
			{
				auto copiedBlock = copyBlock(it->second);
				if (!copiedBlock)
					return false;

				outBlock = *copiedBlock;		
			}
		}
		else
		{
			if (!producerNode->getBlock(producerCursor, this, mixer, outBlock))
				return false;

			auto copiedBlock = copyBlock(outBlock);
			if (!copiedBlock)
				return false;

			m_cachedBlocks[producerPin] = *copiedBlock;
		}
	}

	return true;
}

bool GraphEvaluator::evaluateBlock(const InputPin* consumerPin, const IAudioMixer* mixer, AudioBlock& outBlock) const
{
	const OutputPin* producerPin = m_graph->findSourcePin(consumerPin);
	if (producerPin)
		return evaluateBlock(producerPin, mixer, outBlock);
	else
		return false;
}

NodePinType GraphEvaluator::evaluatePinType(const InputPin* consumerPin) const
{
	const OutputPin* producerPin = m_graph->findSourcePin(consumerPin);
	return producerPin ? producerPin->getPinType() : NodePinType::Void;
}

float GraphEvaluator::getTime() const
{
	return float(m_timer.getElapsedTime());
}

void GraphEvaluator::flushCachedBlocks()
{
	for (auto& block : m_blocks)
	{
		for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
		{
			if (block.samples[i])
				Alloc::freeAlign(block.samples[i]);
		}
	}
	m_blocks.clear();
	m_cachedBlocks.reset();
}

AudioBlock* GraphEvaluator::copyBlock(const AudioBlock& sourceBlock) const
{
	if (m_blocks.full())
		return nullptr;

	AudioBlock& block = m_blocks.push_back();

	for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
	{
		if (sourceBlock.samples[i])
		{
			block.samples[i] = (float*)Alloc::acquireAlign(alignUp(sourceBlock.samplesCount, 4) * sizeof(float), 16, T_FILE_LINE);

			const float* sp = sourceBlock.samples[i];
			float* dp = block.samples[i];

			int32_t j = 0;
			for (; j < (int32_t)sourceBlock.samplesCount - 16; j += 4 * 4)
			{
				const Vector4 s0 = Vector4::loadAligned(sp); sp += 4;
				const Vector4 s1 = Vector4::loadAligned(sp); sp += 4;
				const Vector4 s2 = Vector4::loadAligned(sp); sp += 4;
				const Vector4 s3 = Vector4::loadAligned(sp); sp += 4;
				
				s0.storeAligned(dp); dp += 4;
				s1.storeAligned(dp); dp += 4;
				s2.storeAligned(dp); dp += 4;
				s3.storeAligned(dp); dp += 4;
			}
			for (; j < (int32_t)sourceBlock.samplesCount; j += 4)
			{
				const Vector4 s0 = Vector4::loadAligned(sp); sp += 4;
				s0.storeAligned(dp); dp += 4;
			}
		}
		else
			block.samples[i] = nullptr;
	}

	block.samplesCount = sourceBlock.samplesCount;
	block.sampleRate = sourceBlock.sampleRate;
	block.maxChannel = sourceBlock.maxChannel;
	block.category = sourceBlock.category;

	return &block;
}

}
