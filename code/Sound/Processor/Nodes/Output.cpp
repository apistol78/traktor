#include "Sound/ISoundBuffer.h"
#include "Sound/Processor/GraphEvaluator.h"
#include "Sound/Processor/Nodes/Output.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const ImmutableNode::InputPinDesc c_Output_i[] = { { L"Input", NptSignal, false }, { 0 } };

class OutputCursor : public RefCountImpl< ISoundBufferCursor >
{
public:
	virtual void setParameter(handle_t id, float parameter) override final {}

	virtual void disableRepeat() override final {}

	virtual void reset() override final {}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Output", 0, Output, ImmutableNode)

Output::Output()
:	ImmutableNode(c_Output_i, nullptr)
{
}

bool Output::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

Ref< ISoundBufferCursor > Output::createCursor() const 
{
	Ref< OutputCursor > outputCursor = new OutputCursor();
	return outputCursor;
}

bool Output::getScalar(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const
{
	return false;
}

bool Output::getBlock(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, const ISoundMixer* mixer, SoundBlock& outBlock) const
{
	return evaluator->evaluateBlock(getInputPin(0), mixer, outBlock);
}

	}
}