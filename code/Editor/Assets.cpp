#include "Editor/Assets.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.editor.Assets", 0, Assets, ISerializable)

void Assets::serialize(ISerializer& s)
{
	s >> MemberStlVector< Guid >(L"dependencies", m_dependencies);
}

	}
}
