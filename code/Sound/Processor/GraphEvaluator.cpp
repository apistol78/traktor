#include "Core/Log/Log.h"
#include "Sound/Processor/Graph.h"
#include "Sound/Processor/GraphEvaluator.h"
#include "Sound/Processor/Node.h"
#include "Sound/Processor/OutputPin.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.GraphEvaluator", GraphEvaluator, Object)

bool GraphEvaluator::create(const Graph* graph)
{
	m_graph = graph;
	for (auto node : graph->getNodes())
	{
		Ref< ISoundBufferCursor > nodeCursor = node->createCursor();
		if (!nodeCursor)
		{
			log::error << L"Node \"" << type_name(node) << L"\" failed; no cursor." << Endl;
			return false;
		}

		m_nodeCursors[node] = nodeCursor;
	}
	m_timer.start();
	return true;
}

bool GraphEvaluator::evaluateScalar(const OutputPin* producerPin, float& outScalar) const
{
	const Node* producerNode = producerPin->getNode();
	T_ASSERT(producerNode != nullptr);

	ISoundBufferCursor* producerCursor = m_nodeCursors[producerNode];
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

bool GraphEvaluator::evaluateBlock(const OutputPin* producerPin, const IAudioMixer* mixer, SoundBlock& outBlock) const
{
	const Node* producerNode = producerPin->getNode();
	T_ASSERT(producerNode != nullptr);

	ISoundBufferCursor* producerCursor = m_nodeCursors[producerNode];
	if (!producerCursor)
		return false;

	if (!producerNode->getBlock(producerCursor, this, mixer, outBlock))
		return false;

	return true;
}

bool GraphEvaluator::evaluateBlock(const InputPin* consumerPin, const IAudioMixer* mixer, SoundBlock& outBlock) const
{
	const OutputPin* producerPin = m_graph->findSourcePin(consumerPin);
	if (producerPin)
		return evaluateBlock(producerPin, mixer, outBlock);
	else
		return false;
}

float GraphEvaluator::getTime() const
{
	return float(m_timer.getElapsedTime());
}

	}
}
