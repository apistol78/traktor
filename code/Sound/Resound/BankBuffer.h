#ifndef traktor_sound_BankBuffer_H
#define traktor_sound_BankBuffer_H

#include <vector>
#include "Resource/Proxy.h"
#include "Sound/ISoundBuffer.h"

namespace traktor
{
	namespace sound
	{

class Sound;

class BankBuffer : public ISoundBuffer
{
	T_RTTI_CLASS;

public:
	BankBuffer(std::vector< resource::Proxy< Sound > >& sounds);

	virtual Ref< ISoundBufferCursor > createCursor();

	virtual bool getBlock(const ISoundBufferCursor* cursor, SoundBlock& outBlock);

private:
	std::vector< resource::Proxy< Sound > > m_sounds;
};

	}
}

#endif	// traktor_sound_BankBuffer_H
