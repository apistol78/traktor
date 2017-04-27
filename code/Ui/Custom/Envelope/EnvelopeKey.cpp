/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/Envelope/EnvelopeKey.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.EnvelopeKey", EnvelopeKey, Object)

EnvelopeKey::EnvelopeKey(float T, float value, bool fixedT, bool fixedValue)
:	m_T(T)
,	m_value(value)
,	m_fixedT(fixedT)
,	m_fixedValue(fixedValue)
{
}

		}
	}
}
