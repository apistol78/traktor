/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/RuntimeException.h"

namespace traktor
{

RuntimeException::RuntimeException(const std::wstring& what_)
:	m_what(what_)
{
}

const std::wstring& RuntimeException::what() const throw()
{
	return m_what;
}

}
