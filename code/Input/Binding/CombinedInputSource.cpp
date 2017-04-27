/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/StringOutputStream.h"
#include "Input/Binding/CombinedInputSource.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.CombinedInputSource", CombinedInputSource, IInputSource)

CombinedInputSource::CombinedInputSource(const RefArray< IInputSource >& sources, CombineMode mode)
:	m_sources(sources)
,	m_mode(mode)
{
}

std::wstring CombinedInputSource::getDescription() const
{
	StringOutputStream ss;
	for (RefArray< IInputSource >::const_iterator i = m_sources.begin(); i != m_sources.end(); ++i)
	{
		if (!ss.empty())
		{
			switch (m_mode)
			{
			case CmAny:
				ss << L", " << Endl;
				break;

			case CmExclusive:
				ss << L" ^ " << Endl;
				break;

			case CmAll:
				ss << L" + " << Endl;
				break;
			}
		}
		ss << (*i)->getDescription();
	}
	return ss.str();
}

void CombinedInputSource::prepare(float T, float dT)
{
	for (RefArray< IInputSource >::iterator i = m_sources.begin(); i != m_sources.end(); ++i)
		(*i)->prepare(T, dT);
}

float CombinedInputSource::read(float T, float dT)
{
	bool value = false;

	switch (m_mode)
	{
	case CmAny:
		{
			for (RefArray< IInputSource >::iterator i = m_sources.begin(); i != m_sources.end(); ++i)
				value |= asBoolean((*i)->read(T, dT));
		}
		break;

	case CmExclusive:
		{
			uint32_t count = 0;
			for (RefArray< IInputSource >::iterator i = m_sources.begin(); i != m_sources.end(); ++i)
			{
				if (asBoolean((*i)->read(T, dT)))
					++count;
			}
			value = bool(count == 1);
		}
		break;

	case CmAll:
		{
			value = true;
			for (RefArray< IInputSource >::iterator i = m_sources.begin(); i != m_sources.end(); ++i)
				value &= asBoolean((*i)->read(T, dT));
		}
		break;
	}

	return asFloat(value);
}

	}
}
