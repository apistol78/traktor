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

class Pattern;

class T_DLLCLASS SongBuffer : public ISoundBuffer
{
	T_RTTI_CLASS;

public:
	SongBuffer(const RefArray< Pattern >& patterns, int32_t bpm);

	virtual Ref< ISoundBufferCursor > createCursor() const override final;

	virtual bool getBlock(ISoundBufferCursor* cursor, const IAudioMixer* mixer, SoundBlock& outBlock) const override final;

	int32_t getCurrentPattern(const ISoundBufferCursor* cursor) const;

	int32_t getCurrentRow(const ISoundBufferCursor* cursor) const;

private:
	RefArray< Pattern > m_patterns;
	int32_t m_bpm;
};

	}
}
