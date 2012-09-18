#include "Core/Math/MathUtils.h"
#include "Parade/Network/IReplicatableState.h"
#include "Parade/Network/StateProphet.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

const float c_maxExtrapolate = 1.0f + 2.0f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.StateProphet", StateProphet, Object)

void StateProphet::push(float T, IReplicatableState* state)
{
	// Ensure state is valid before adding to buffer.
	if (!m_history.empty())
	{
		if (!m_history.back().state->verify(state))
			return;
	}

	History h;
	h.T = T;
	h.state = state;
	m_history.push_back(h);
}

Ref< const IReplicatableState > StateProphet::get(float T) const
{
	if (m_history.empty())
		return 0;

	if (m_history.size() <= 1)
		return m_history.back().state;

	uint32_t N = m_history.size();

	if (T >= m_history[N - 1].T)
	{
		// Linear predict state from last two.
		float dST = m_history[N - 1].T - m_history[N - 2].T;
		float Tc = 1.0f + (T - m_history[N - 1].T) / dST;

		if (Tc < 1.0f)
			Tc = 1.0f;
		else if (Tc >= c_maxExtrapolate)
			return 0;

		return m_history[N - 2].state->extrapolate(m_history[N - 1].state, Tc);
	}
	else
	{
		// Interpolate state between two known states.
		for (size_t i = 0; i < N - 1; ++i)
		{
			if (T >= m_history[i].T && T <= m_history[i + 1].T)
			{
				float dST = m_history[i + 1].T - m_history[i].T;
				float Tc = (T - m_history[i].T) / dST;
				return m_history[i].state->extrapolate(m_history[i + 1].state, Tc);
			}
		}
	}

	// Should not happen; something wrong in the above.
	T_ASSERT_M(0, L"Failed to see past, presence or future");
	return 0;
}

	}
}
