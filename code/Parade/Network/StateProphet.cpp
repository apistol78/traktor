#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Parade/Network/IReplicatableState.h"
#include "Parade/Network/StateProphet.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

const float c_maxExtrapolate = 1.0f;	//< Means a maximum of 1 second(s) latency.

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.StateProphet", StateProphet, Object)

void StateProphet::push(float T, IReplicatableState* state)
{
	// Ensure state is valid before adding to buffer.
	if (!m_history.empty() && !m_history.back().state->verify(state))
		return;

	// Do not add too old state.
	if (!m_history.empty() && T <= m_history.front().T)
		return;

	// Add state.
	History h;
	h.T = T;
	h.state = state;
	m_history.push_back(h);

	// Ensure history list is sorted based on time.
	int32_t swapped = 0;
	do
	{
		swapped = 0;
		for (uint32_t i = 0; i < m_history.size() - 1; ++i)
		{
			if (m_history[i].T > m_history[i + 1].T)
			{
				std::swap(m_history[i], m_history[i + 1]);
				++swapped;
			}
		}
	}
	while (swapped > 0);
}

Ref< const IReplicatableState > StateProphet::get(float T) const
{
	uint32_t N = m_history.size();

	if (!N)
		return 0;

	if (N <= 1)
		return m_history.back().state;

	if (T >= m_history[N - 1].T)
	{
		if (T - m_history[N - 1].T >= c_maxExtrapolate)
			return 0;

		if (N > 2)
			return m_history[N - 3].state->extrapolate(
				m_history[N - 3].T,
				m_history[N - 2].T, m_history[N - 2].state,
				m_history[N - 1].T, m_history[N - 1].state,
				T
			);
		else
			return m_history[N - 2].state->extrapolate(
				m_history[N - 2].T,
				m_history[N - 1].T, m_history[N - 1].state,
				T
			);
	}
	else
	{
		for (size_t i = 0; i < N - 1; ++i)
		{
			if (T >= m_history[i].T && T <= m_history[i + 1].T)
			{
				return m_history[i].state->extrapolate(
					m_history[i].T,
					m_history[i + 1].T, m_history[i + 1].state,
					T
				);
			}
		}
	}

	return 0;
}

	}
}
