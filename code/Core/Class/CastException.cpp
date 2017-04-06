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
