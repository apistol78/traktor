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

	virtual void reset();

	virtual bool getBlock(double time, SoundBlock& outBlock);

private:
	std::vector< resource::Proxy< Sound > > m_sounds;
	Ref< Sound > m_activeSound;
};

	}
}

#endif	// traktor_sound_BankBuffer_H
