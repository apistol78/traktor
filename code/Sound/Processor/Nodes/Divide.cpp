#include "Sound/ISoundBuffer.h"
#include "Sound/Processor/GraphEvaluator.h"
#include "Sound/Processor/Nodes/Divide.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const ImmutableNode::InputPinDesc c_Divide_i[] =
{
	{ L"Input1", NptScalar, false },
	{ L"Input2", NptScalar, false },
	{ 0 }
};

const ImmutableNode::OutputPinDesc c_Divide_o[] =
{
	{ L"Output", NptSignal },
	{ 0 }
};

class DivideCursor : public RefCountImpl< ISoundBufferCursor >
{
public:
	virtual void setParameter(handle_t id, float parameter) override final {}

	virtual void disableRepeat() override final {}

	virtual void reset() override final {}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Divide", 0, Divide, ImmutableNode)

Divide::Divide()
:	ImmutableNode(c_Divide_i, c_Divide_o)
{
}

bool Divide::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

Ref< ISoundBufferCursor > Divide::createCursor() const 
{
	return new DivideCursor();
}

bool Divide::getScalar(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const
{
	float lh, rh;
	if (!evaluator->evaluateScalar(getInputPin(0), lh))
		return false;
	if (!evaluator->evaluateScalar(getInputPin(1), rh))
		return false;

	outScalar = lh / rh;
	return true;
}

bool Divide::getBlock(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, const ISoundMixer* mixer, SoundBlock& outBlock) const
{
	return false;
}

	}
}