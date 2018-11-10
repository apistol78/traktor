#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/Processor/Nodes/Scalar.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const ImmutableNode::OutputPinDesc c_Scalar_o[] =
{
	{ L"Output", NptScalar },
	{ 0 }
};

class ScalarCursor : public RefCountImpl< ISoundBufferCursor >
{
public:
	virtual void setParameter(handle_t id, float parameter) T_OVERRIDE T_FINAL {}

	virtual void disableRepeat() T_OVERRIDE T_FINAL {}

	virtual void reset() T_OVERRIDE T_FINAL {}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Scalar", 0, Scalar, ImmutableNode)

Scalar::Scalar()
:	ImmutableNode(nullptr, c_Scalar_o)
,	m_value(0.0f)
{
}

bool Scalar::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

Ref< ISoundBufferCursor > Scalar::createCursor() const 
{
	return new ScalarCursor();
}

bool Scalar::getScalar(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const
{
	outScalar = m_value;
	return true;
}

bool Scalar::getBlock(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, const ISoundMixer* mixer, SoundBlock& outBlock) const
{
	return false;
}

void Scalar::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	s >> Member< float >(L"value", m_value);
}

	}
}