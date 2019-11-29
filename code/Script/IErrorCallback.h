#pragma once

#include <string>

namespace traktor
{
    namespace script
    {

/*! Script error callback.
 * \ingroup Script
 */
class IErrorCallback
{
public:
	virtual ~IErrorCallback() {}

	/*! Syntax error callback. */
	virtual void syntaxError(const std::wstring& name, uint32_t line, const std::wstring& message) = 0;

	/*! Other error callback. */
	virtual void otherError(const std::wstring& message) = 0;
};

    }
}
