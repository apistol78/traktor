#include "Runtime/Target/CommandEvent.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.CommandEvent", 0, CommandEvent, IRemoteEvent)

CommandEvent::CommandEvent(const std::wstring& function)
:	m_function(function)
{
}

const std::wstring& CommandEvent::getFunction() const
{
	return m_function;
}

void CommandEvent::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"function", m_function);
}

	}
}
