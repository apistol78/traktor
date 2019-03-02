#include "Net/Replication/State/IValue.h"
#include "Net/Replication/State/State.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.State", State, Object)

State::State()
:	m_index(0)
{
}

State::State(const RefArray< const IValue >& values)
:	m_values(values)
,	m_index(0)
{
}

void State::packBegin()
{
	m_values.resize(0);
}

void State::pack(const IValue* value)
{
	m_values.push_back(value);
}

void State::unpackBegin()
{
	m_index = 0;
}

const IValue* State::unpack()
{
	T_ASSERT (m_index < m_values.size());
	return m_values[m_index++];
}

	}
}
