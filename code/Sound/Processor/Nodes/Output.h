#pragma once

#include "Sound/Processor/ImmutableNode.h"

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

class T_DLLCLASS Output : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Output();

	virtual bool bind(resource::IResourceManager* resourceManager) T_OVERRIDE T_FINAL;

	virtual Ref< ISoundBufferCursor > createCursor() const T_OVERRIDE T_FINAL;

	virtual bool getScalar(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const T_OVERRIDE T_FINAL;
	
	virtual bool getBlock(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, const ISoundMixer* mixer, SoundBlock& outBlock) const T_OVERRIDE T_FINAL;
};

	}
}
