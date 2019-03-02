#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Script/IScriptBlob.h"
#include "Script/ScriptResource.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptResource", 0, ScriptResource, ISerializable)

void ScriptResource::serialize(ISerializer& s)
{
	s >> MemberStlVector< Guid >(L"dependencies", m_dependencies);
	s >> MemberRef< const IScriptBlob >(L"blob", m_blob);
}

	}
}
