#ifndef traktor_net_BodyStateValue_H
#define traktor_net_BodyStateValue_H

#include "Net/Replication/State/IValue.h"
#include "Physics/BodyState.h"

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

class T_DLLCLASS BodyStateValue : public IValue
{
	T_RTTI_CLASS;

public:
	typedef physics::BodyState value_t;
	typedef const physics::BodyState& value_argument_t;
	typedef physics::BodyState value_return_t;

	BodyStateValue()
	{
	}

	BodyStateValue(const physics::BodyState& value)
	:	m_value(value)
	{
	}

	operator const physics::BodyState& () const { return m_value; }

	virtual void serialize(ISerializer& s);

private:
	physics::BodyState m_value;
};

	}
}

#endif	// traktor_net_BodyStateValue_H
