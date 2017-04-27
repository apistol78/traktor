/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/Envelope/EnvelopeContentChangeEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.EnvelopeContentChangeEvent", EnvelopeContentChangeEvent, ContentChangeEvent)

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
}
