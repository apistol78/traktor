#include "Amalgam/FrameProfiler.h"

namespace traktor
{
	namespace amalgam
	{

void FrameProfiler::beginFrame()
{
	m_markers.resize(0);
	m_stack.resize(0);
	m_timer.start();
}

void FrameProfiler::endFrame()
{
	Marker m =
	{
		FptEndFrame,
		0,
		0.0,
		m_timer.getElapsedTime()
	};
	m_markers.push_back(m);
}

void FrameProfiler::beginScope(uint8_t id)
{
	Marker m =
	{
		id,
		uint8_t(m_stack.size()),
		m_timer.getElapsedTime(),
		0.0
	};
	m_markers.push_back(m);
	m_stack.push_back(m_markers.size() - 1);
}

void FrameProfiler::endScope()
{
	T_ASSERT (!m_stack.empty());

	int32_t index = m_stack.back();
	m_stack.pop_back();

	T_ASSERT (index >= 0);
	T_ASSERT (index < m_markers.size());

	m_markers[index].end = m_timer.getElapsedTime();
}

	}
}
