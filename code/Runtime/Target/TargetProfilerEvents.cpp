/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStaticVector.h"
#include "Runtime/Target/TargetProfilerEvents.h"

namespace traktor::runtime
{
	namespace
	{

class MemberProfilerEvent : public MemberComplex
{
public:
	MemberProfilerEvent(const wchar_t* const name, Profiler::Event& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		s >> Member< uint16_t >(L"name", m_ref.name);
		s >> Member< uint8_t >(L"threadId", m_ref.threadId);
		s >> Member< uint8_t >(L"depth", m_ref.depth);
		s >> Member< double >(L"start", m_ref.start);
		s >> Member< double >(L"end", m_ref.end);
	}

private:
	Profiler::Event& m_ref;
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.TargetProfilerEvents", 0, TargetProfilerEvents, ISerializable)

TargetProfilerEvents::TargetProfilerEvents(double currentTime, const Profiler::eventQueue_t& events)
:	m_currentTime(currentTime)
,	m_events(events)
{
}

void TargetProfilerEvents::serialize(ISerializer& s)
{
	s >> Member< double >(L"currentTime", m_currentTime);
	s >> MemberStaticVector< Profiler::Event, Profiler::MaxQueuedEvents, MemberProfilerEvent >(L"events", m_events);
}

}
