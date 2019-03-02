#include "Ui/Envelope/EnvelopeKey.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.EnvelopeKey", EnvelopeKey, Object)

EnvelopeKey::EnvelopeKey(float T, float value, bool fixedT, bool fixedValue)
:	m_T(T)
,	m_value(value)
,	m_fixedT(fixedT)
,	m_fixedValue(fixedValue)
{
}

	}
}
