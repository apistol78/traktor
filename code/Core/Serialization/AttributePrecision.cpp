/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/AttributePrecision.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.AttributePrecision", AttributePrecision, Attribute)

AttributePrecision::AttributePrecision(PrecisionType precision)
:	m_precision(precision)
{
}

AttributePrecision::PrecisionType AttributePrecision::getPrecision() const
{
	return m_precision;
}

}
