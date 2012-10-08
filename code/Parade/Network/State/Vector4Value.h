#ifndef traktor_parade_Vector4Value_H
#define traktor_parade_Vector4Value_H

#include "Core/Math/Vector4.h"
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

class T_DLLCLASS Vector4Value : public IValue
{
	T_RTTI_CLASS;

public:
	typedef Vector4 value_t;
	typedef const Vector4& value_argument_t;
	typedef Vector4 value_return_t;

	Vector4Value(const Vector4& value)
	:	m_value(value)
	{
	}

	operator Vector4 () const { return m_value; }

private:
	Vector4 m_value;
};

	}
}

#endif	// traktor_parade_Vector4Value_H
