#include "Sound/ISoundBuffer.h"
#include "Sound/Processor/GraphEvaluator.h"
#include "Sound/Processor/Nodes/Time.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const ImmutableNode::OutputPinDesc c_Time_o[] =
{
	{ L"Output", NptSignal },
	{ 0 }
};

class TimeCursor : public RefCountImpl< ISoundBufferCursor >
{
public:
	virtual void setParameter(handle_t id, float parameter) T_OVERRIDE T_FINAL {}

	virtual void disableRepeat() T_OVERRIDE T_FINAL {}

	virtual void reset() T_OVERRIDE T_FINAL {}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Time", 0, Time, ImmutableNode)

Time::Time()
:	ImmutableNode(nullptr, c_Time_o)
{
}

bool Time::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

Ref< ISoundBufferCursor > Time::createCursor() const 
{
	return new TimeCursor();
}

bool Time::getScalar(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const
{
	outScalar = evaluator->getTime();
	return true;
}

bool Time::getBlock(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, const ISoundMixer* mixer, SoundBlock& outBlock) const
{
	return false;
}

	}
}