#include "Input/Binding/CombinedInputSource.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.CombinedInputSource", CombinedInputSource, Object)

CombinedInputSource::CombinedInputSource(IInputSource* source1, IInputSource* source2)
:	m_source1(source1)
,	m_source2(source2)
{
}

std::wstring CombinedInputSource::getDescription() const
{
	if (m_source1 && m_source2)
		return m_source1->getDescription() + L" + " + m_source2->getDescription();
	else if (m_source1)
		return m_source1->getDescription();
	else if (m_source2)
		return m_source2->getDescription();
	else
		return L"";
}

float CombinedInputSource::read(InputSystem* inputSystem, float T, float dT)
{
	float value1 = m_source1 ? m_source1->read(inputSystem, T, dT) : 1.0f;
	float value2 = m_source2 ? m_source2->read(inputSystem, T, dT) : 1.0f;
	return asFloat(asBoolean(value1) & asBoolean(value2));
}

	}
}
