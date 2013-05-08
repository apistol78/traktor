#include "Database/Local/LocalFileLink.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.LocalFileLink", 0, LocalFileLink, ISerializable)

void LocalFileLink::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"path", m_path);
}

	}
}
