#pragma once

#include "Core/RefArray.h"
#include "Sound/ISoundBuffer.h"

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

/*! \brief GraphBuffer instance.
 */
class T_DLLCLASS GraphBuffer : public ISoundBuffer
{
	T_RTTI_CLASS;

public:
	GraphBuffer(const Graph* graph);

	virtual Ref< ISoundBufferCursor > createCursor() const T_OVERRIDE T_FINAL;

	virtual bool getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const T_OVERRIDE T_FINAL;

private:
	Ref< const Graph > m_graph;
};

	}
}