#include "Core/Log/Log.h"
#include "Sound/Processor/Graph.h"
#include "Sound/Processor/GraphBuffer.h"
#include "Sound/Processor/GraphEvaluator.h"
#include "Sound/Processor/Nodes/Output.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

class GraphBufferCursor : public RefCountImpl< ISoundBufferCursor >
{
public:
	Ref< GraphEvaluator > m_evaluator;
	Ref< const InputPin > m_outputPin;

	virtual void setParameter(handle_t id, float parameter) T_OVERRIDE T_FINAL {}

	virtual void disableRepeat() T_OVERRIDE T_FINAL {}

	virtual void reset() T_OVERRIDE T_FINAL {}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.GraphBuffer", GraphBuffer, Object)

GraphBuffer::GraphBuffer(const Graph* graph)
:	m_graph(graph)
{
}

Ref< ISoundBufferCursor > GraphBuffer::createCursor() const
{
	Ref< GraphBufferCursor > graphCursor = new GraphBufferCursor();

	graphCursor->m_evaluator = new GraphEvaluator();
	if (!graphCursor->m_evaluator->create(m_graph))
	{
		log::error << L"Unable to create graph evaluator." << Endl;
		return nullptr;
	}

	for (auto node : m_graph->getNodes())
	{
		if (is_a< Output >(node))
		{
			graphCursor->m_outputPin = node->getInputPin(0);
			break;
		}
	}
	if (!graphCursor->m_outputPin)
	{
		log::error << L"Unable to find output pin." << Endl;
		return nullptr;
	}

	return graphCursor;
}

bool GraphBuffer::getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const
{
	GraphBufferCursor* graphCursor = static_cast< GraphBufferCursor* >(cursor);
	return graphCursor->m_evaluator->evaluateBlock(
		graphCursor->m_outputPin,
		mixer,
		outBlock
	);
}

	}
}
