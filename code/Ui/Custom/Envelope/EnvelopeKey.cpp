#include "Ui/Custom/Envelope/EnvelopeKey.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.EnvelopeKey", EnvelopeKey, Object)

EnvelopeKey::EnvelopeKey(float T, float value) :
	m_T(T),
	m_value(value)
{
}

void EnvelopeKey::setT(float T)
{
	m_T = T;
}

float EnvelopeKey::getT() const
{
	return m_T;
}

void EnvelopeKey::setValue(float value)
{
	m_value = value;
}

float EnvelopeKey::getValue() const
{
	return m_value;
}

		}
	}
}
