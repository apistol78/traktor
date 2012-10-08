#ifndef traktor_parade_QuaternionValue_H
#define traktor_parade_QuaternionValue_H

#include "Core/Math/Quaternion.h"
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

class T_DLLCLASS QuaternionValue : public IValue
{
	T_RTTI_CLASS;

public:
	typedef Quaternion value_t;
	typedef const Quaternion& value_argument_t;
	typedef Quaternion value_return_t;

	QuaternionValue(const Quaternion& value)
	:	m_value(value)
	{
	}

	operator Quaternion () const { return m_value; }

private:
	Quaternion m_value;
};

	}
}

#endif	// traktor_parade_QuaternionValue_H
