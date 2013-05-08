#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Script/CallStack.h"
#include "Script/Local.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

class MemberFrame : public MemberComplex
{
public:
	MemberFrame(const wchar_t* const name, CallStack::Frame& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		s >> Member< Guid >(L"scriptId", m_ref.scriptId);
		s >> Member< std::wstring >(L"scriptName", m_ref.scriptName);
		s >> Member< std::wstring >(L"functionName", m_ref.functionName);
		s >> Member< uint32_t >(L"line", m_ref.line);
		s >> MemberRefArray< Local >(L"locals", m_ref.locals);
	}

private:
	CallStack::Frame& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.CallStack", 0, CallStack, ISerializable)

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
	s >> MemberStlList< Frame, MemberFrame >(L"frames", m_frames);
}

CallStack::Frame::Frame()
:	line(0)
{
}

	}
}
