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
