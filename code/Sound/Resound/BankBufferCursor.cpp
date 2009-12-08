#include "Sound/Resound/BankBufferCursor.h"

namespace traktor
{
	namespace sound
	{

BankBufferCursor::BankBufferCursor(ISoundBuffer* activeBuffer, ISoundBufferCursor* activeCursor)
:	m_activeBuffer(activeBuffer)
,	m_activeCursor(activeCursor)
{
}

void BankBufferCursor::setCursor(double time)
{
	m_activeCursor->setCursor(time);
}

	}
}
