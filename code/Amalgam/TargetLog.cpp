#include "Amalgam/TargetLog.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.TargetLog", 0, TargetLog, ISerializable)

TargetLog::TargetLog()
:	m_level(0)
{
}

TargetLog::TargetLog(int32_t level, const std::wstring& text)
:	m_level(level)
,	m_text(text)
{
}

void TargetLog::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"level", m_level);
	s >> Member< std::wstring >(L"text", m_text);
}

	}
}
