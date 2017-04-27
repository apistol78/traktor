/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/String.h"
#include "Input/Binding/ConstantInputSource.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.ConstantInputSource", ConstantInputSource, IInputSource)

ConstantInputSource::ConstantInputSource(float value)
:	m_value(value)
{
}

std::wstring ConstantInputSource::getDescription() const
{
	return toString(m_value);
}

void ConstantInputSource::prepare(float T, float dT)
{
}

float ConstantInputSource::read(float T, float dT)
{
	return m_value;
}

	}
}
