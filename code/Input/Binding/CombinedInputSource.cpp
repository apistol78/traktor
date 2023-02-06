/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/StringOutputStream.h"
#include "Input/Binding/CombinedInputSource.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor::input
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
	for (auto source : m_sources)
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
		ss << source->getDescription();
	}
	return ss.str();
}

void CombinedInputSource::prepare(float T, float dT)
{
	for (auto source : m_sources)
		source->prepare(T, dT);
}

float CombinedInputSource::read(float T, float dT)
{
	bool value = false;

	switch (m_mode)
	{
	case CmAny:
		{
			for (auto source : m_sources)
				value |= asBoolean(source->read(T, dT));
		}
		break;

	case CmExclusive:
		{
			uint32_t count = 0;
			for (auto source : m_sources)
			{
				if (asBoolean(source->read(T, dT)))
					++count;
			}
			value = bool(count == 1);
		}
		break;

	case CmAll:
		{
			value = true;
			for (auto source : m_sources)
				value &= asBoolean(source->read(T, dT));
		}
		break;
	}

	return asFloat(value);
}

}
