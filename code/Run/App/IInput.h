#pragma once

#include <string>
#include "Core/Object.h"

namespace traktor
{
	namespace run
	{

/*! Scriptable input reader interface.
 * \ingroup Run
 */
class IInput : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool endOfFile() = 0;

	virtual std::wstring readChar() = 0;

	virtual std::wstring readLn() = 0;
};

	}
}

