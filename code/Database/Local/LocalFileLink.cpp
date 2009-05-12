#include "Database/Local/LocalFileLink.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.db.LocalFileLink", LocalFileLink, Serializable)

bool LocalFileLink::serialize(Serializer& s)
{
	return s >> Member< std::wstring >(L"path", m_path);
}

	}
}
