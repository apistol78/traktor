#ifndef traktor_net_TransformValue_H
#define traktor_net_TransformValue_H

#include "Core/Math/Transform.h"
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

class T_DLLCLASS TransformValue : public IValue
{
	T_RTTI_CLASS;

public:
	typedef Transform value_t;
	typedef const Transform& value_argument_t;
	typedef Transform value_return_t;

	TransformValue(const Transform& value)
	:	m_value(value)
	{
	}

	operator const Transform& () const { return m_value; }

private:
	Transform m_value;
};

	}
}

#endif	// traktor_net_TransformValue_H
