/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/CastException.h"

namespace traktor
{

CastException::CastException(const char* what_)
:	m_what(what_)
{
}

const char* CastException::what() const throw()
{
	return m_what;
}

}
