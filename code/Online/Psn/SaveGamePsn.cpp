#include "Online/Psn/SaveGamePsn.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SaveGamePsn", SaveGamePsn, ISaveGame)

SaveGamePsn::SaveGamePsn(const std::wstring& name, ISerializable* attachment)
:	m_name(name)
,	m_attachment(attachment)
{
}

std::wstring SaveGamePsn::getName() const
{
	return m_name;
}

Ref< ISerializable > SaveGamePsn::getAttachment() const
{
	return m_attachment;
}

	}
}
