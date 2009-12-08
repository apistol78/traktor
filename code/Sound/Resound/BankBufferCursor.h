#ifndef traktor_sound_BankBufferCursor_H
#define traktor_sound_BankBufferCursor_H

#include "Sound/ISoundBuffer.h"

namespace traktor
{
	namespace sound
	{

class BankBufferCursor : public RefCountImpl< ISoundBufferCursor >
{
public:
	BankBufferCursor(ISoundBuffer* activeBuffer, ISoundBufferCursor* activeCursor);

	virtual void setCursor(double time);

	ISoundBuffer* getActiveBuffer() const { return m_activeBuffer; }

	ISoundBufferCursor* getActiveCursor() const { return m_activeCursor; }

private:
	Ref< ISoundBuffer > m_activeBuffer;
	Ref< ISoundBufferCursor > m_activeCursor;
};

	}
}

#endif	// traktor_sound_BankBufferCursor_H
