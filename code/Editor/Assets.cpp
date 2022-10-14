#include "Editor/Assets.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.editor.Assets", 1, Assets, ISerializable)

void Assets::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion() >= 1);
	s >> MemberStlVector< Dependency, MemberComposite< Dependency > >(L"dependencies", m_dependencies);
}

void Assets::Dependency::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"id", id);
	s >> Member< bool >(L"editorDeployOnly", editorDeployOnly);
}

}
