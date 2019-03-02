#include "Ui/Envelope/EnvelopeContentChangeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.EnvelopeContentChangeEvent", EnvelopeContentChangeEvent, ContentChangeEvent)

EnvelopeContentChangeEvent::EnvelopeContentChangeEvent(EventSubject* sender, EnvelopeKey* key)
:	ContentChangeEvent(sender)
,	m_key(key)
{
}

EnvelopeKey* EnvelopeContentChangeEvent::getKey() const
{
	return m_key;
}

	}
}
