#pragma once

#include "Resource/IdProxy.h"
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

class Sound;

class T_DLLCLASS Source : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Source();

	virtual bool bind(resource::IResourceManager* resourceManager) T_OVERRIDE T_FINAL;

	virtual Ref< ISoundBufferCursor > createCursor() const T_OVERRIDE T_FINAL;

	virtual bool getScalar(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const T_OVERRIDE T_FINAL;
	
	virtual bool getBlock(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, const ISoundMixer* mixer, SoundBlock& outBlock) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const resource::IdProxy< Sound >& getSound() const { return m_sound; }

private:
	resource::IdProxy< Sound > m_sound;
};

	}
}
