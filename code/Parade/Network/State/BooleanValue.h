#ifndef traktor_parade_BooleanValue_H
#define traktor_parade_BooleanValue_H

#include "Parade/Network/State/IValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PARADE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace parade
	{

class T_DLLCLASS BooleanValue : public IValue
{
	T_RTTI_CLASS;

public:
	typedef bool value_t;
	typedef bool value_argument_t;
	typedef bool value_return_t;

	BooleanValue(bool value)
	:	m_value(value)
	{
	}

	operator bool () const { return m_value; }

private:
	bool m_value;
};

	}
}

#endif	// traktor_parade_BooleanValue_H
