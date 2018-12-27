#include "Sound/ISoundBuffer.h"
#include "Sound/Processor/GraphEvaluator.h"
#include "Sound/Processor/Nodes/Add.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const ImmutableNode::InputPinDesc c_Add_i[] =
{
	{ L"Input1", NptScalar, false },
	{ L"Input2", NptScalar, false },
	{ 0 }
};

const ImmutableNode::OutputPinDesc c_Add_o[] =
{
	{ L"Output", NptSignal },
	{ 0 }
};

class AddCursor : public RefCountImpl< ISoundBufferCursor >
{
public:
	virtual void setParameter(handle_t id, float parameter) override final {}

	virtual void disableRepeat() override final {}

	virtual void reset() override final {}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Add", 0, Add, ImmutableNode)

Add::Add()
:	ImmutableNode(c_Add_i, c_Add_o)
{
}

bool Add::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

Ref< ISoundBufferCursor > Add::createCursor() const 
{
	return new AddCursor();
}

bool Add::getScalar(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const
{
	float lh, rh;
	if (!evaluator->evaluateScalar(getInputPin(0), lh))
		return false;
	if (!evaluator->evaluateScalar(getInputPin(1), rh))
		return false;

	outScalar = lh + rh;
	return true;
}

bool Add::getBlock(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, const ISoundMixer* mixer, SoundBlock& outBlock) const
{
	return false;
}

	}
}