#pragma once

#include <functional>
#include <vector>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEBBER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace wbr
	{

class T_DLLCLASS Event : public Object
{
	T_RTTI_CLASS;

public:
	void raise();

	void operator += (const std::function< void() >& fn);

private:
	std::vector< std::function< void() > > m_fns;
};

	}
}

