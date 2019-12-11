#pragma once

#include <string>
#include "Core/Object.h"

namespace traktor
{
	namespace run
	{

/*! Scriptable output writer interface.
 * \ingroup Run
 */
class IOutput : public Object
{
	T_RTTI_CLASS;

public:
	virtual void print(const std::wstring& s) = 0;

	virtual void printLn(const std::wstring& s) = 0;
};

	}
}

