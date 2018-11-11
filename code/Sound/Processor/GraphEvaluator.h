#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Timer/Timer.h"

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

class Graph;
class InputPin;
class ISoundBufferCursor;
class ISoundMixer;
class Node;
class OutputPin;
struct SoundBlock;

class T_DLLCLASS GraphEvaluator : public Object
{
	T_RTTI_CLASS;

public:
	bool create(const Graph* graph);

	bool evaluateScalar(const OutputPin* producerPin, float& outScalar) const;

	bool evaluateScalar(const InputPin* consumerPin, float& outScalar) const;

	bool evaluateBlock(const OutputPin* producerPin, const ISoundMixer* mixer, SoundBlock& outBlock) const;

	bool evaluateBlock(const InputPin* consumerPin, const ISoundMixer* mixer, SoundBlock& outBlock) const;

	float getTime() const;

private:
	Ref< const Graph > m_graph;
	SmallMap< const Node*, Ref< ISoundBufferCursor > > m_nodeCursors;
	Timer m_timer;
};

	}
}
