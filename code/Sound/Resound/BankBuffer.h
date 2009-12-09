#ifndef traktor_sound_BankBuffer_H
#define traktor_sound_BankBuffer_H

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

class IGrain;
class BankSound;

class T_DLLCLASS BankBuffer : public ISoundBuffer
{
	T_RTTI_CLASS;

public:
	BankBuffer(
		const RefArray< IGrain >& grains,
		const RefArray< BankSound >& sounds
	);

	BankSound* getSound(int32_t index) const;

	virtual Ref< ISoundBufferCursor > createCursor() const;

	virtual bool getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const;

private:
	RefArray< IGrain > m_grains;
	RefArray< BankSound > m_sounds;
};

	}
}

#endif	// traktor_sound_BankBuffer_H
