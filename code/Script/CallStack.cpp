#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberStl.h"
#include "Script/CallStack.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

class MemberLocal : public MemberComplex
{
public:
	MemberLocal(const wchar_t* const name, CallStack::Local& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		s >> Member< std::wstring >(L"name", m_ref.name);
		s >> Member< std::wstring >(L"value", m_ref.value);
		return true;
	}

private:
	CallStack::Local& m_ref;
};

class MemberFrame : public MemberComplex
{
public:
	MemberFrame(const wchar_t* const name, CallStack::Frame& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		s >> Member< Guid >(L"scriptId", m_ref.scriptId);
		s >> Member< std::wstring >(L"scriptName", m_ref.scriptName);
		s >> Member< std::wstring >(L"functionName", m_ref.functionName);
		s >> Member< uint32_t >(L"line", m_ref.line);
		s >> MemberStlList< CallStack::Local, MemberLocal >(L"locals", m_ref.locals);
		return true;
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

bool CallStack::serialize(ISerializer& s)
{
	return s >> MemberStlList< Frame, MemberFrame >(L"frames", m_frames);
}

CallStack::Frame::Frame()
:	line(0)
{
}

	}
}
