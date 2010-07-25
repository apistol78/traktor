#include "Core/Io/StringOutputStream.h"
#include "Input/Binding/CombinedInputSource.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.CombinedInputSource", CombinedInputSource, Object)

CombinedInputSource::CombinedInputSource(const RefArray< IInputSource >& sources)
:	m_sources(sources)
{
}

std::wstring CombinedInputSource::getDescription() const
{
	StringOutputStream ss;
	for (RefArray< IInputSource >::const_iterator i = m_sources.begin(); i != m_sources.end(); ++i)
	{
		if (!ss.empty())
			ss << L" + ";
		ss << (*i)->getDescription();
	}
	return ss.str();
}

float CombinedInputSource::read(InputSystem* inputSystem, float T, float dT)
{
	for (RefArray< IInputSource >::iterator i = m_sources.begin(); i != m_sources.end(); ++i)
	{
		bool value = asBoolean((*i)->read(inputSystem, T, dT));
		if (!value)
			return asFloat(false);
	}
	return asFloat(true);
}

	}
}
