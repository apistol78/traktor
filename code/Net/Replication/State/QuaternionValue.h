#ifndef traktor_net_QuaternionValue_H
#define traktor_net_QuaternionValue_H

#include "Core/Math/Quaternion.h"
#include "Net/Replication/State/IValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
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

#endif	// traktor_net_QuaternionValue_H
