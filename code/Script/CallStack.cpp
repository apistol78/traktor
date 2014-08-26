#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Script/CallStack.h"
#include "Script/Local.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.CallStack", 0, CallStack, ISerializable)

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.CallStack.Frame", 0, CallStack::Frame, ISerializable)

void CallStack::pushFrame(const Frame& frame)
{
	m_frames.push_back(frame);
}

bool CallStack::popFrame()
{
	if (!m_frames.empty())
		m_frames.pop_back();
	return !m_frames.empty();
}

const CallStack::Frame& CallStack::getCurrentFrame() const
{
	return m_frames.front();
}

const std::list< CallStack::Frame >& CallStack::getFrames() const
{
	return m_frames;
}

void CallStack::serialize(ISerializer& s)
{
	s >> MemberStlList< Frame, MemberComposite< Frame > >(L"frames", m_frames);
}

CallStack::Frame::Frame()
:	line(0)
{
}

void CallStack::Frame::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"scriptId", scriptId);
	s >> Member< std::wstring >(L"scriptName", scriptName);
	s >> Member< std::wstring >(L"functionName", functionName);
	s >> Member< uint32_t >(L"line", line);
	s >> MemberRefArray< Local >(L"locals", locals);
}

	}
}
