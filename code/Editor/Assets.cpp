#include "Editor/Assets.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.editor.Assets", Assets, Serializable)

bool Assets::serialize(Serializer& s)
{
	return s >> MemberStlVector< Guid >(L"dependencies", m_dependencies);
}

	}
}
